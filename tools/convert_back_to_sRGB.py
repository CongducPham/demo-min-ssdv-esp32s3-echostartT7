import sys
import cv2

# Load the Y′CbCr image
ycbcr_image = cv2.imread(sys.argv[1]+".jpg")  # Assuming it's in YCrCb format

# Convert Y′CbCr back to BGR (which is equivalent to sRGB in OpenCV)
srgb_image = cv2.cvtColor(ycbcr_image, cv2.COLOR_YCrCb2BGR)  # OpenCV uses YCrCb notation

# Save the result
cv2.imwrite(sys.argv[1]+"_restored_srgb.jpg", srgb_image)