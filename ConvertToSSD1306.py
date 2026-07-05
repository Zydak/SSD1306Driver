#!/usr/bin/env python3

import sys
import subprocess
import tempfile
import shutil
from PIL import Image, ImageSequence
import os

def get_average_brightness(img):
    gray = img.convert('L')
    pixels = list(gray.getdata())
    return sum(pixels) // len(pixels) if pixels else 128


def image_to_ssd1306_bytes(img, width, height, threshold):
    gray = img.convert('L')
    pages = (height + 7) // 8
    data = bytearray(pages * width)
    
    for y in range(height):
        page = y // 8
        bit = y % 8
        for x in range(width):
            if gray.getpixel((x, y)) >= threshold:
                data[page * width + x] |= (1 << bit)
    return data


def extract_frames_from_video(video_path):
    """Extract frames from MP4"""
    temp_dir = tempfile.mkdtemp()
    frame_pattern = os.path.join(temp_dir, "frame_%06d.png")
    
    try:
        cmd = ['ffmpeg', '-i', video_path, '-vf', 'fps=10', frame_pattern, '-loglevel', 'quiet']
        subprocess.run(cmd, check=True)
        
        frame_paths = [os.path.join(temp_dir, f) for f in sorted(os.listdir(temp_dir)) if f.endswith('.png')]
        return frame_paths, temp_dir
    except Exception as e:
        print(f"FFmpeg error: {e}")
        shutil.rmtree(temp_dir, ignore_errors=True)
        return [], None


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 convert_gif_to_ssd1306.py <input.(gif|mp4|png|jpg|jpeg|webp)> [width] [height] [output.h]")
        print("Example:")
        print("   python3 convert_gif_to_ssd1306.py animation.mp4 128 64")
        print("   python3 convert_gif_to_ssd1306.py logo.png 128 64")
        sys.exit(1)

    input_path = sys.argv[1]
    print(f"Processing: {input_path}")

    out_width = None
    out_height = None
    h_path = None
    idx = 2

    if idx < len(sys.argv) and sys.argv[idx].isdigit():
        out_width = int(sys.argv[idx])
        idx += 1
    if idx < len(sys.argv) and sys.argv[idx].isdigit():
        out_height = int(sys.argv[idx])
        idx += 1
    if idx < len(sys.argv):
        h_path = sys.argv[idx]

    if not h_path:
        base = os.path.splitext(os.path.basename(input_path))[0]
        suffix = f"_{out_width}x{out_height}" if (out_width and out_height) else ""
        h_path = f"{base}{suffix}.h"

    try:
        is_video = input_path.lower().endswith(('.mp4', '.mov', '.avi'))
        frame_list = []
        temp_dir = None

        # === Handle different input types ===
        if is_video:
            print("Extracting frames from video...")
            frame_list, temp_dir = extract_frames_from_video(input_path)
            if not frame_list:
                print("Failed to extract video frames")
                sys.exit(1)
        else:
            # Static image or GIF
            with Image.open(input_path) as im:
                if hasattr(im, 'n_frames') and im.n_frames > 1:
                    # Animated GIF
                    print(f"Processing animated GIF with {im.n_frames} frames...")
                    for i in range(im.n_frames):
                        im.seek(i)
                        frame_list.append(im.copy())
                else:
                    # Static image (PNG, JPG, etc.)
                    print("Processing static image...")
                    frame_list.append(im.copy())

        # === Convert all frames ===
        frames_data = []
        frame_count = 0
        orig_w, orig_h = None, None

        for item in frame_list:
            if isinstance(item, str):  # video frame path
                img = Image.open(item)
            else:  # PIL Image
                img = item

            if orig_w is None:
                orig_w, orig_h = img.size

            img = img.convert('RGB')
            if out_width and out_height:
                img = img.resize((out_width, out_height), Image.Resampling.LANCZOS)
            
            w = out_width or orig_w
            h = out_height or orig_h
            
            avg = get_average_brightness(img)
            threshold = max(70, min(200, avg))
            
            byte_data = image_to_ssd1306_bytes(img, w, h, threshold)
            frames_data.append(byte_data)
            frame_count += 1

        if not frames_data:
            print("No frames found")
            sys.exit(1)

        frame_size = len(frames_data[0])
        var_name = os.path.splitext(os.path.basename(input_path))[0].replace("-", "_").replace(".", "_")

        with open(h_path, 'w') as f:
            f.write(f"#ifndef {var_name.upper()}_H\n")
            f.write(f"#define {var_name.upper()}_H\n\n")
            f.write(f"// Generated from {input_path}\n")
            if out_width and out_height:
                f.write(f"// Resized to {out_width}x{out_height}\n")
            f.write(f"// {frame_count} frames, {w}x{h} pixels\n")
            f.write(f"// Each frame is {frame_size} bytes\n\n")

            f.write(f"const int {var_name}FrameCount = {frame_count};\n")
            f.write(f"const int {var_name}FrameSize = {frame_size};\n\n")

            f.write(f"const uint8_t {var_name}Data[{frame_count}][{frame_size}] = {{\n")
            for i, frame_data in enumerate(frames_data):
                f.write("    {")
                for j in range(0, frame_size, 16):
                    chunk = frame_data[j:j+16]
                    hex_bytes = ', '.join(f"0x{b:02X}" for b in chunk)
                    if j + 16 < frame_size:
                        f.write(f"\n        {hex_bytes},")
                    else:
                        f.write(f"\n        {hex_bytes}")
                f.write("\n    }")
                if i < frame_count - 1:
                    f.write(",")
                f.write("\n")
            f.write("};\n\n")
            f.write("#endif\n")

        print(f"Generated {h_path} with {frame_count} frame(s)")
        if temp_dir:
            shutil.rmtree(temp_dir, ignore_errors=True)

    except Exception as e:
        print(f"Error: {e}")
        if 'temp_dir' in locals() and temp_dir:
            shutil.rmtree(temp_dir, ignore_errors=True)


if __name__ == "__main__":
    main()