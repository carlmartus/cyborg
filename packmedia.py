import sys, glob, os

if __name__ == '__main__':
	header = open(sys.argv[3], 'w')


	header.write("// WARNING AUTOGENERATED\n")
	header.write("#define MEDIA_FILE \"%s\"\n" % sys.argv[2])
	offset = 0
	output = open(sys.argv[2], 'wb')

	mediaDir = sys.argv[1]
	search = '%s/*.*' % mediaDir

	for f in glob.glob(search):
		fileName = f[len(mediaDir)+1:]
		define = 'FILE_%s' % fileName.split('.')[0].upper()

		fd = open(f, 'rb')
		buf = fd.read()
		fd.close()

		output.write(buf)

		header.write("// WARNING AUTOGENERATED\n")
		header.write("#define %s %d\n" % (define, offset))
		header.write("#define %s_LEN %d\n" % (define, len(buf)))
		offset += len(buf)

	output.close()
	header.write("// WARNING AUTOGENERATED\n")
	header.close()

