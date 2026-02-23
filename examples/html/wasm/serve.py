import http.server
import socketserver

PORT = 8000

class HttpRequestHandler(http.server.SimpleHTTPRequestHandler):
    extensions_map = {
        '': 'application/octet-stream',
        '.manifest': 'text/cache-manifest',
        '.html': 'text/html',
        '.png': 'image/png',
        '.jpg': 'image/jpg',
        '.svg':	'image/svg+xml',
        '.css':	'text/css',
        '.js':'application/x-javascript',
		'.mjs':'text/javascript',
        '.wasm': 'application/wasm',
        '.json': 'application/json',
        '.xml': 'application/xml',
    }

httpd = socketserver.TCPServer(("localhost", PORT), HttpRequestHandler)

try:
    print(f"serving at http://localhost:{PORT}")
    with socketserver.TCPServer(("", PORT), HttpRequestHandler) as httpd:
        print("serving at port", PORT)
        httpd.serve_forever()
except KeyboardInterrupt:
    pass