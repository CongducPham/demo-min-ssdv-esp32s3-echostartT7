import sys
import cv2

# Load the image in sRGB space
image = cv2.imread(sys.argv[1]+".jpg")

# Convert to Y′CbCr color space
ycbcr_image = cv2.cvtColor(image, cv2.COLOR_BGR2YCrCb)  # OpenCV uses YCrCb notation

# Save the result
if len(sys.argv) > 2:
  cv2.imwrite(sys.argv[1]+"Q"+sys.argv[2]+"_yCbCr.jpg", ycbcr_image, [int(cv2.IMWRITE_JPEG_QUALITY), int(sys.argv[2])])
else:
  cv2.imwrite(sys.argv[1]+"_yCbCr.jpg", ycbcr_image)