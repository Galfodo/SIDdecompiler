import os
import sys
import numpy as np

psid_fields = [
  ('id', 'S4'),
  ('version', '>u2'),
  ('dataOffset', '>u2'),
  ('loadAddress', '>u2'),
  ('initAddress', '>u2'),
  ('playAddress', '>u2'),
  ('songs', '>u2'),
  ('startSong', '>u2'),
  ('speed', '>u4'),
  ('name', 'S32'),
  ('author', 'S32'),
  ('released', 'S32') 
  ]

rsid_fields = [ ]  
  
psid_hdr_type = np.dtype(psid_fields)
rsid_hdr_type = list(psid_fields)
rsid_hdr_type.extend(rsid_fields)

def set_subtunes(contents, count):
    contents[0x0F] = int(count)
    
options = { \
  "subtunes": set_subtunes,
  "s": set_subtunes
}

def load_sid(filename):
    raw = np.fromfile(filename, dtype=np.uint8)
    hdr = raw[0:psid_hdr_type.itemsize].view(dtype=psid_hdr_type)
    return raw, hdr

def main():
    apply_opts = []
    file = None
    for i in range(1, len(sys.argv)):
        if sys.argv[i].startswith("-"):
            opt = sys.argv[i][1:]
            name, value = opt.split("=")
            if name in options.keys():
                func = options[name]
                apply_opts.append((func, value))
            else:
                print("Unknown option {}".format(name))
                return -1
        else:
            file = sys.argv[i]
    print(file)

    raw, header = load_sid(file)

    for opt in apply_opts:
        func, param = opt
        func(raw, param)

    
    fields = sorted(header.dtype.fields.items(), key = lambda x: x[1][1])
    for field, type_ in fields:
        value = header[field][0]
        if not type_[0].kind == 'S':
            value = hex(value)
        print("{:20}: {}".format(field, value))
#    if raw[5] == 0x01:
#        print("PSID V1")
#
#        with open(files[:-4]+".prg", mode='wb') as file:
#            file.write(raw[0x76:])
#            file.close()
#
#    elif raw[5] in [0x02,0x03]:
#        print("PSID/RSID V2/V3")
#
#       with open(files[:-4]+".prg", mode='wb') as file:
#           file.write(raw[0x7c:])
#           file.close()
#    
#    with open(file[:-4]+"_patched.sid", mode="wb") as file:
#        file.write(raw)

if __name__ == "__main__":
    main()
