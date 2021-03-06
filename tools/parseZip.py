import struct,sys

def extractCdh(zipFile):
    indList = [0]
    sign = bytes(b"\x50\x4b\x01\x02")
    while True:
        try:
            indList.append(zipFile.index(sign, indList[-1]+1, len(zipFile)))
        except:
            break
    indList = indList[1:]
    indList.append(len(zipFile))
    return [zipFile[indList[i]:indList[i+1]] for i in range(len(indList)-1)]

def createCdhEntry(cdh_entry):
	d = {}
	d['signature'] = cdh_entry[0:4]
	d['version'] = struct.unpack("<h", cdh_entry[4:6])[0]
	d['version_min'] = struct.unpack("<h", cdh_entry[6:8])[0]
	d['flags'] = struct.unpack("<h", cdh_entry[8:10])[0]
	d['compression'] = struct.unpack("<h", cdh_entry[10:12])[0]
	d['mod_time'] = struct.unpack("<h", cdh_entry[12:14])[0]
	d['mod_date'] = struct.unpack("<h", cdh_entry[14:16])[0]
	d['crc32'] = struct.unpack("I", cdh_entry[16:20])
	d['compressed_size'] = struct.unpack("<I", cdh_entry[20:24])[0]
	d['uncompressed_size'] = struct.unpack("<I", cdh_entry[24:28])[0]
	d['filename_length'] = struct.unpack("<h", cdh_entry[28:30])[0]
	d['extra_field_length'] = struct.unpack("<h", cdh_entry[30:32])[0]
	d['comment_length'] = struct.unpack("<h", cdh_entry[32:34])[0]
	d['disk_number'] = struct.unpack("<h", cdh_entry[34:36])[0]
	d['internal_file_attributes'] = struct.unpack("<h", cdh_entry[36:38])[0]
	d['external_file_attributes'] = struct.unpack("<I", cdh_entry[38:42])[0]
	d['offset_file_header'] = struct.unpack("<I", cdh_entry[42:46])[0]
	d['filename'] = cdh_entry[46:46+d['filename_length']].decode() # unicode string
	x = 46+d['filename_length']
	d['extra_field'] = cdh_entry[x:x+d['extra_field_length']]
	x += d['extra_field_length']
	d['file_comment'] = cdh_entry[x:x+d['comment_length']]
	return d


def getCdhEntry(zipname, filename):
    cdh = extractCdh(zipname)
    if filename is None:
        return [createCdhEntry(entry) for entry in cdh]
    return [createCdhEntry(entry) for entry in cdh if createCdhEntry(entry)['filename'] == filename][0]

if __name__ == "__main__":
        if len(sys.argv) < 3:
                print("Usage: python %s 'zipname' 'filename'" % sys.argv[0])
                exit(1)
        zipname = open(sys.argv[1], 'rb').read()
        print("Central directory:")
        cdh = extractCdh(zipname)
        for entry in cdh:
	        print(createCdhEntry(entry))
        print()
        print("CRC32 of %s: %s" % (sys.argv[2], str(getCdhEntry(zipname, sys.argv[2])['crc32'])[1:-2]))
