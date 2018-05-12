#set -x
for file in file1 file2 etc
do
  echo "processing $file.gif"
  convert "$file"'.gif[0]' $file-frame.gif
  gifsicle --resize 20x20 "$file"-frame.gif > "$file"-ico.gif
  rm "$file"-frame.gif
  # convert the animated gif
  echo "  processing $file.RAW"
  ffmpeg -i $file.gif -r 22 -f rawvideo -s 128x128 -pix_fmt rgb565be $file.RAW
  # convert the preview
  echo "  processing $file.PRV"
  ffmpeg -i $file-sm.gif -vframes 1 -r 22 -f rawvideo -s 101x112 -pix_fmt rgb565be $file.PRV
  # Convert the icon
  echo "  processing $file.ICO"
  ffmpeg -i $file-ico.gif -vframes 1 -r 22 -f rawvideo -s 20x20 -pix_fmt rgb565be $file.ICO
done
