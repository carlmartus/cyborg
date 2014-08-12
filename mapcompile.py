import sys
from struct import pack

CONV = {
		' ' : 0,
		'X' : 1,
		'Z' : 8,

		'W' : 4,
		'_' : 5,
		'.' : 6,
		':' : 7,
		}

if __name__ == '__main__':
	w, h = 0, 0
	lines = []
	for line in open(sys.argv[1], 'r'):
		line = line.strip()
		if len(line) > w: w = len(line)
		lines.append(line.strip())
	h = len(lines)

	arr = []
	for l in lines:
		i = 0
		vec = []
		for c in l:
			vec.append(c)
			i += 1
		for j in range(i, w):
			vec.append(' ')
		arr.append(vec)

	# Write
	output = open(sys.argv[2], 'wb')
	output.write(pack('2H', w, h))

	for row in arr:
		for cell in row:
			con = CONV[cell]
			output.write(pack('H', con))
	output.close()


