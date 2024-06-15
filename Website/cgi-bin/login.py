import cgi
import sys

def main():
		
	form = cgi.FieldStorage()

	email = form['email'].value if 'email' in form else None
	password = form['password'].value if 'password' in form else None

	if not email or not password:
		sys.exit("Error: Email and password are required")

	html_content = """<html>
	<head>
	<title>Hello, World!</title>
	</head>
	<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH" crossorigin="anonymous">
	<body>
	<div class="container col-xl-10 col-xxl-8 px-4 py-5 text-center">
	<h1 class="h3 m-3 fw-normal">Login Successful</h1>
	<p>
	Email: {email}<br>
	Password: {password}<br>
	</p>
	</div>
	</body>
	</html>"""

	html_content = html_content.format(email=email, password=password)

	print("HTTP/1.1 200 OK")
	print("Content-type: text/html; charset=utf-8")
	print("Content-length:", len(html_content.encode('utf-8')))
	print()
	print(html_content)


if __name__ == "__main__":
    main()
