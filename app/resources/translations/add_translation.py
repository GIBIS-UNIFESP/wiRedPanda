import xml.etree.ElementTree as ET
tree = ET.parse('wpanda_es.ts')
root = tree.getroot()
with open('es.txt') as f:
    lines = f.read().splitlines()
for idx,log_element in enumerate(root.findall('.//translation')):
  log_element.text = lines[idx]
  #print(log_element.text)
with open('teste.xml', 'wb') as f2:
    tree.write(f2)
