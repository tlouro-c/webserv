#! /usr/bin/env python3

import os
import sys

def main():

	if "CONTENT_LENGTH" in os.environ and int(os.environ["CONTENT_LENGTH"]) > 0:
		body = sys.stdin.read(int(os.environ["CONTENT_LENGTH"]))
	else:
		body = ""
		
	html_content = f"""<html>
	<head>
	<title>Body Output Page</title>
	<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
	</head>
	<body>
	<div class="container col-xl-10 col-xxl-8 px-4 py-5 text-center">
	<p>{body}</p>
	</div>
	</body>
	</html>"""

	print("HTTP/1.1 200 OK")
	print("Content-type: text/html")
	print("Content-length:", len(html_content.encode('utf-8')))
	print()
	print(html_content)

if __name__ == "__main__":
	main()
