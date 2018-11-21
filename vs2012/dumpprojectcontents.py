
import glob
import xml.etree.ElementTree

def getprojectfiles(filename):
	from xml.dom import minidom
	xmldoc = minidom.parse(filename)
	sources = 			[s.attributes['Include'].value.replace('\\', '/') for s in xmldoc.getElementsByTagName('ClCompile') if 'Include' in s.attributes.keys()]
	headers = 			[s.attributes['Include'].value.replace('\\', '/') for s in xmldoc.getElementsByTagName('ClInclude') if 'Include' in s.attributes.keys()]
	dependencies = 	[s.attributes['Include'].value.replace('\\', '/') for s in xmldoc.getElementsByTagName('ProjectReference') if 'Include' in s.attributes.keys()]
	return (sources, headers, dependencies)

def dumplist(l):
	for i in l:
		print ' \\'
		print str('  "' + i + '"').ljust(60),
	print
	print
		
def dumpprojectfiles(filename):
		srcs,hdrs,deps = getprojectfiles(filename)
		deps = [d.replace('.vcxproj', '') for d in deps]
		prjname = filename.replace('.vcxproj', '').upper()
		print str(prjname + '_SOURCES = ').ljust(60),
		dumplist(srcs)
		if len(hdrs):
			print str(prjname + '_HEADERS = ').ljust(60),
			dumplist(hdrs)
		if len(deps):
			print str(prjname + '_DEPENDENCIES = ').ljust(60),
			dumplist(deps)
		print

def main():
	files = glob.glob('*.vcxproj')
	for f in files:
		dumpprojectfiles(f)
		
if __name__ == "__main__":
	main()
	