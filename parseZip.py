import re,struct,sys

def extractCdh(zipFile):
	indList = [0]
	while True:
		try:
			indList.append(zipFile.index("\x50\x4b\x01\x02", indList[-1]+1, len(zipFile))) 
		except:
			break
	indList = indList[1:]
	indList.append(len(zipFile))
	return [zipFile[indList[i]:indList[i+1]] for i in range(len(indList)-1)]
		
def createCdhEntry(cdh_entry):
	d = {}
	d['signature'] = cdh_entry[0:4].encode('hex')
	d['version'] = struct.unpack("<h", cdh_entry[4:6])[0]
	d['version_min'] = struct.unpack("<h", cdh_entry[6:8])[0]
	d['flags'] = struct.unpack("<h", cdh_entry[8:10])[0]
	d['compression'] = struct.unpack("<h", cdh_entry[10:12])[0]
	d['mod_time'] = struct.unpack("<h", cdh_entry[12:14])[0]
	d['mod_date'] = struct.unpack("<h", cdh_entry[14:16])[0]
	d['crc32'] = cdh_entry[16:20].encode('hex')
	d['compressed_size'] = struct.unpack("<I", cdh_entry[20:24])[0]
	d['uncompressed_size'] = struct.unpack("<I", cdh_entry[24:28])[0]
	d['filename_length'] = struct.unpack("<h", cdh_entry[28:30])[0]
	d['extra_field_length'] = struct.unpack("<h", cdh_entry[30:32])[0]
	d['comment_length'] = struct.unpack("<h", cdh_entry[32:34])[0]
	d['disk_number'] = struct.unpack("<h", cdh_entry[34:36])[0]
	d['internal_file_attributes'] = struct.unpack("<h", cdh_entry[36:38])[0]
	d['external_file_attributes'] = struct.unpack("<I", cdh_entry[38:42])[0]
	d['offset_file_header'] = struct.unpack("<I", cdh_entry[42:46])[0]
	d['filename'] = cdh_entry[46:46+d['filename_length']]
	x = 46+d['filename_length']
	d['extra_field'] = cdh_entry[x:x+d['extra_field_length']]
	x += d['extra_field_length']
	d['file_comment'] = cdh_entry[x:x+d['comment_length']]
	return d


def printCdh(f):
	cdh = extractCdh(f)
	for entry in cdh:
		print createCdhEntry(entry)
	

if __name__ == "__main__":
	printCdh(open(sys.argv[1]).read())
