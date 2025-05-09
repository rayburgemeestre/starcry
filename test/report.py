#!/usr/bin/env python3
import os
import sys
import datetime
import subprocess
import base64
from io import BytesIO
from pathlib import Path
from PIL import Image, ImageChops, ImageDraw, ImageFont
import numpy as np
import datetime
import argparse

def get_image_datetime(image_path):
    """Get the creation datetime of the image file."""
    try:
        timestamp = os.path.getmtime(image_path)
        return datetime.datetime.fromtimestamp(timestamp)
    except Exception as e:
        return "Unknown"

def get_image_filesize(image_path):
    """Get the file size of the image in a human-readable format."""
    size_in_bytes = os.path.getsize(image_path)
    
    # Convert to human-readable format
    for unit in ['B', 'KB', 'MB', 'GB']:
        if size_in_bytes < 1024.0:
            return f"{size_in_bytes:.2f} {unit}"
        size_in_bytes /= 1024.0
    
    return f"{size_in_bytes:.2f} TB"

def compare_images(img1_path, img2_path):
    """Compare two images and return the RMSE error, similar to the C++ implementation."""
    # Use ImageMagick's compare tool like in the C++ code
    diff_path = "/tmp/diff.png"
    output_path = "/tmp/diff.txt"
    
    cmd = f"compare -metric RMSE {img1_path} {img2_path} {diff_path} 2> {output_path}"
    subprocess.run(cmd, shell=True)
    
    # Read the error value
    try:
        with open(output_path, 'r') as f:
            output = f.read()
        
        # Extract the RMSE value
        import re
        match = re.search(r'[\d.]+ \(([\d.-]+(?:e[\d.-]+)?)\)', output)
        if match:
            rmse = float(match.group(1))
        else:
            rmse = 1.0  # Default value as in the C++ code
    except Exception as e:
        rmse = 1.0
    
    return rmse

def create_diff_image(img1_path, img2_path):
    """Create a visual difference image that highlights pixel differences.
    Returns the PIL Image object instead of saving to a file."""
    try:
        img1 = Image.open(img1_path)
        img2 = Image.open(img2_path)
        
        # Make sure images are the same size and mode
        if img1.size != img2.size:
            img2 = img2.resize(img1.size)
        
        if img1.mode != img2.mode:
            img2 = img2.convert(img1.mode)
        
        # Convert images to RGB if they aren't already
        img1_rgb = img1.convert('RGB')
        img2_rgb = img2.convert('RGB')
        
        # Get numpy arrays from images
        arr1 = np.array(img1_rgb)
        arr2 = np.array(img2_rgb)
        
        # Calculate absolute difference
        diff_array = np.abs(arr1.astype(np.int16) - arr2.astype(np.int16))
        
        # Sum across RGB channels to get overall difference at each pixel
        diff_sum = np.sum(diff_array, axis=2)
        
        # Create a mask of significant differences
        threshold = 10  # Adjust threshold as needed
        significant_diff = diff_sum > threshold
        
        # Create a difference visualization
        diff_image = Image.new('RGB', img1.size, (240, 240, 240))  # Light gray background
        draw = ImageDraw.Draw(diff_image)
        
        # Draw red pixels where significant differences exist
        for y in range(img1.height):
            for x in range(img1.width):
                if significant_diff[y, x]:
                    # Red color for differences
                    draw.point((x, y), fill=(255, 0, 0))
        
        # Add a border
        draw.rectangle([(0, 0), (img1.width-1, img1.height-1)], outline=(0, 0, 0), width=1)
        
        return diff_image
    except Exception as e:
        print(f"Error creating diff image: {e}")
        return None

def image_to_base64(image_path):
    """Convert an image file to a base64 encoded string."""
    if isinstance(image_path, Image.Image):
        # If image_path is already a PIL Image object
        img = image_path
    else:
        # If image_path is a file path
        img = Image.open(image_path)
    
    buffered = BytesIO()
    img.save(buffered, format="PNG")
    return base64.b64encode(buffered.getvalue()).decode()

def generate_report(last_run_dir, reference_dir, output_html):
    """Generate an HTML report comparing images in last_run_dir and reference_dir."""
    last_run_path = Path(last_run_dir)
    reference_path = Path(reference_dir)
    
    last_run_images = {f.name: f for f in last_run_path.glob("*.png")}
    reference_images = {f.name: f for f in reference_path.glob("*.png")}
    
    # Find images present in both directories
    common_images = set(last_run_images.keys()) & set(reference_images.keys())
    
    # HTML report header
    html = """
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Image Comparison Report</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                margin: 20px;
                line-height: 1.6;
            }
            h1, h2 {
                color: #333;
            }
            .report-header {
                margin-bottom: 30px;
            }
            .image-comparison {
                margin-bottom: 50px;
                border-bottom: 1px solid #ccc;
                padding-bottom: 30px;
            }
            .image-row {
                display: flex;
                flex-wrap: wrap;
                justify-content: space-between;
                margin-top: 20px;
            }
            .image-container {
                flex: 0 0 32%;
                margin-bottom: 20px;
                text-align: center;
            }
            .image-container img {
                max-width: 100%;
                height: auto;
                border: 1px solid #ddd;
            }
            .image-title {
                font-weight: bold;
                margin-bottom: 5px;
                font-size: 14px;
            }
            .image-metadata {
                margin-top: 10px;
                font-size: 12px;
                color: #666;
            }
            .error-value {
                font-weight: bold;
                font-size: 16px;
                color: #e44;
            }
            .error-low {
                color: green;
            }
            .error-medium {
                color: orange;
            }
            .error-high {
                color: red;
            }
            table {
                width: 100%;
                border-collapse: collapse;
                margin-top: 10px;
            }
            th, td {
                border: 1px solid #ddd;
                padding: 8px;
                text-align: left;
            }
            th {
                background-color: #f2f2f2;
            }
            .summary {
                margin-top: 30px;
                padding: 15px;
                background-color: #f9f9f9;
                border-radius: 5px;
            }
        </style>
    </head>
    <body>
        <div class="report-header">
            <h1>Image Comparison Report</h1>
            <p>Generated on: """ + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S") + """</p>
            <p>Comparing images from:</p>
            <ul>
                <li>Last Run: """ + str(last_run_path) + """</li>
                <li>Reference: """ + str(reference_path) + """</li>
            </ul>
            <p>Total images compared: """ + str(len(common_images)) + """</p>
        </div>
    """
    
    # Summary table
    html += """
        <div class="summary">
            <h2>Summary</h2>
            <table>
                <tr>
                    <th>Image Name</th>
                    <th>Error Value</th>
                    <th>Last Run Date</th>
                    <th>File Size</th>
                </tr>
    """
    
    # Process each image
    image_data = []
    for image_name in sorted(common_images):
        last_run_image = last_run_images[image_name]
        reference_image = reference_images[image_name]
        
        # Compare images
        error = compare_images(str(last_run_image), str(reference_image))
        
        # Create diff image
        diff_image = create_diff_image(str(last_run_image), str(reference_image))
        
        # Convert images to base64
        last_run_base64 = image_to_base64(last_run_image)
        reference_base64 = image_to_base64(reference_image)
        diff_base64 = image_to_base64(diff_image) if diff_image else ""
        
        # Get metadata
        datetime_str = get_image_datetime(last_run_image)
        filesize = get_image_filesize(last_run_image)
        
        # Add to the list of image data
        image_data.append({
            'name': image_name,
            'error': error,
            'datetime': datetime_str,
            'filesize': filesize,
            'last_run_base64': last_run_base64,
            'reference_base64': reference_base64,
            'diff_base64': diff_base64
        })
    
    # Sort by error value (descending)
    image_data.sort(key=lambda x: x['error'], reverse=True)
    
    # Add rows to summary table
    for data in image_data:
        error_class = "error-low" if data['error'] < 0.01 else ("error-medium" if data['error'] < 0.05 else "error-high")
        html += f"""
            <tr>
                <td>{data['name']}</td>
                <td class="{error_class}">{data['error']:.6f}</td>
                <td>{data['datetime']}</td>
                <td>{data['filesize']}</td>
            </tr>
        """
    
    html += """
            </table>
        </div>
    """
    
    # Detailed image comparisons
    for data in image_data:
        html += f"""
        <div class="image-comparison">
            <h2>Image: {data['name']}</h2>
            <div class="image-metadata">
                <p><strong>Date:</strong> {data['datetime']}</p>
                <p><strong>File Size:</strong> {data['filesize']}</p>
                <p><strong>Error Value:</strong> <span class="error-value">{data['error']:.6f}</span></p>
            </div>
            <div class="image-row">
                <div class="image-container">
                    <div class="image-title">Last Run</div>
                    <img src="data:image/png;base64,{data['last_run_base64']}" alt="Last Run Image">
                </div>
                <div class="image-container">
                    <div class="image-title">Reference</div>
                    <img src="data:image/png;base64,{data['reference_base64']}" alt="Reference Image">
                </div>
                <div class="image-container">
                    <div class="image-title">Difference</div>
                    <img src="data:image/png;base64,{data['diff_base64']}" alt="Difference Image">
                </div>
            </div>
        </div>
        """
    
    # HTML footer
    html += """
    </body>
    </html>
    """
    
    # Write the HTML report
    with open(output_html, 'w') as f:
        f.write(html)
    
    print(f"Report generated successfully: {output_html}")
    return len(common_images)

def main():
    parser = argparse.ArgumentParser(description='Generate image comparison report')
    parser.add_argument('--last-run', default='test/integration/last-run',
                        help='Path to last-run directory (default: test/integration/last-run)')
    parser.add_argument('--reference', default='test/integration/reference',
                        help='Path to reference directory (default: test/integration/reference)')
    parser.add_argument('--output', default='test/report.html',
                        help='Output HTML file path (default: test/report.html)')
    
    args = parser.parse_args()
    
    # Generate the report
    num_images = generate_report(args.last_run, args.reference, args.output)
    
    if num_images == 0:
        print("No common images found in both directories!")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())