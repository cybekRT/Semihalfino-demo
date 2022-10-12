#!/usr/bin/env python3

class Font():
	def __init__(self):
		self.width = 0
		self.bytes = []

fontData = {}

with open("font.dat") as f:
	#for line in f.readlines():
	#	print("Line: {}".format(line))
	# xxx = False
	while True:
		line = f.readline()
		if not line:
			break

		character = line[0]
		values = []
		for a in range(0,7):
			value = f.readline().replace("\n", "")
			values.append(value)

		obj = Font()
		obj.width = len(values[0])

		# print("WTF: {}".format(obj))

		for a in range(0, obj.width):
			byte = 0
			for b in range(0, 7):
				# print("{} - {} {}".format(line, a, b))
				if values[b][a] == '1':
					byte = byte | (1 << b)
			# print("b: {}".format(byte))
			obj.bytes.append(byte)

		fontData[ord(character)] = obj
		# print("Line: {} - {}".format(line, values))
		# print("Char: {} = {}".format(character, ord(character)))

		# if xxx is True:
		# 	break
		# xxx = True

with open("font_gen.cpp", "w") as out:
	out.write('#include "Font.hpp"\n\n')
	out.write('FontNode font[] = {')
	for a in range(0, 128):
		if a in fontData:
			out.write("\t{{ .width = {}, .font = {} }},\n".format(fontData[a].width, str(fontData[a].bytes).replace("[", "{").replace("]", "}")))
		else:
			out.write("\t{ .width = 0 },\n")
	out.write('};\n')