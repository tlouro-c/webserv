#! /usr/bin/env python3

import cgi
import os
import sys

def main():

	form = cgi.FieldStorage()

	upload_directory = os.environ['UPLOAD_DIR']
	domain = "http://" + os.environ['DOMAIN']
	if 'filename' in form:
		fileitem = form['filename']
		if fileitem.filename:
			if not upload_directory.endswith('/'):
				upload_directory += '/'
			file_path = upload_directory + fileitem.filename
			with open(file_path, 'wb') as file:
				file.write(fileitem.file.read())

	html_content = """<html>
	<head>
	<title>Hello, World!</title>
	<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
	</head>
	<body>
	<div class="container col-xl-10 col-xxl-8 px-4 py-5 text-center">
	<h2>"""

	if 'filename' in form and fileitem.filename:
		html_content += f'File was saved at <a href="{domain + "/" + upload_directory}" style="text-decoration: underline;"> {domain + "/" + upload_directory} </a>'
	else:
		html_content += "No file was submitted"

	html_content += """</h2>
	</div>
	</body>
	</html>"""

	print("HTTP/1.1 200 OK")
	print("Content-type: text/html; charset=utf-8")
	print("Content-length:", len(html_content.encode('utf-8')))
	print()
	print(html_content)


if __name__ == "__main__":
	main()
