import xml.etree.ElementTree as ET
tree = ET.parse('wpanda_es.ts')
root = tree.getroot()
for idx,log_element in enumerate(root.findall('.//source')):
  print(log_element.text)
