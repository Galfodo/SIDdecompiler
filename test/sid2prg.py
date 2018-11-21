import os
import sys

if len(sys.argv) < 2:
  print "You must supply a *.sid file"
else:
  files = sys.argv[1]
  print files
  
  with open(files, mode='rb') as file:
      fileContent = file.read()

  if  ord(fileContent[5]) == 0x01:
      print "PSID V1"

      with open(files[:-4]+".prg", mode='wb') as file:
          file.write(fileContent[0x76:])
          file.close()

  elif  ord(fileContent[5]) in [0x02,0x03]:
      print "PSID/RSID V2/V3"

      with open(files[:-4]+".prg", mode='wb') as file:
          file.write(fileContent[0x7c:])
          file.close()