# -*- coding: utf-8 -*-
import os
import xml.etree.ElementTree as ET
from bs4 import BeautifulSoup as bs
from jinja2 import Environment, FileSystemLoader, select_autoescape
# from natsort import natsorted, ns

env = Environment(
	loader      = FileSystemLoader(os.path.dirname(os.path.abspath(__file__)) + "/templates/"),
	trim_blocks = True,
	autoescape  = select_autoescape(['html'])
)

# env = Environment(
#     loader=PackageLoader('templates'),
#     autoescape=select_autoescape(['html', 'xml'])
# )

def depth_iter(element, tag=None):
    stack = []
    stack.append(iter([element]))
    while stack:
        e = next(stack[-1], None)
        if e == None:
            stack.pop()
        else:
            stack.append(iter(e))
            if tag == None or e.tag == tag:
                yield (e, len(stack) - 1)

def to_int(value):
	return int(value)

def profile_export(filepath, output):
	if (not os.path.isfile(filepath)):
		print("Profile dump file does not exist. Searched for '{}'".format(os.path.abspath(filepath)))
		return False
	
	template = env.get_template('export_template.html')
	env.globals['to_int'] = to_int
	env.globals['max'] = max
	
	try:
		root = ET.parse(filepath).getroot()
	except ET.ParseError as e:
		print("XML parsing the profile dump failed: {}".format(e))
		return False
	except:
		print("Reading profile dump failed for unknown reason.")
		return False
		
	data = {}
	
	for thread_element in root:
		threadId = int(thread_element.attrib['id'])
		if (threadId != 0):
			continue
		
		data[threadId] = {
			'name': thread_element.attrib['name'],
			'frames': [],
		}
		
		frames = data[threadId]['frames']
		
		for frame_element in thread_element.findall('frame'):
			frame = {
				'start': int(frame_element.attrib['start']),
				'elapsed': int(frame_element.attrib['elapsed']),
				'events': {},
			}
			if (frame['elapsed'] <= 10):
				continue
			
			events = frame['events']
			
			for event_element, level in depth_iter(frame_element, 'event'):
				level = level - 2
				
				if (not level in events):
					events[level] = []
				
				events[level].append({
					'name': event_element.attrib['name'],
					'start': int(event_element.attrib['start']),
					'elapsed': int(event_element.attrib['elapsed']),
				})
		
			frames.append(frame)
	
	with open(output, 'w') as f:
		output_html = template.render(
			data=data,
			render_scale=0.1
		)
		
		f.write(bs(output_html, features="lxml").prettify())
	
	print("BLINGBLONG")

filepath = "../../workdir/profile_lite.xml"
profile_export(filepath, "output.html")
