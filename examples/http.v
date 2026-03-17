# http.v – HTTP/Web constants
# Use these to conditionally compile networking features.

# HTTP methods
val HTTP_GET    = 1
val HTTP_POST   = 2
val HTTP_PUT    = 3
val HTTP_DELETE = 4

# Common HTTP status codes
val HTTP_OK                    = 200
val HTTP_CREATED               = 201
val HTTP_BAD_REQUEST           = 400
val HTTP_UNAUTHORIZED          = 401
val HTTP_FORBIDDEN             = 403
val HTTP_NOT_FOUND             = 404
val HTTP_INTERNAL_SERVER_ERROR = 500

# Default server settings
val HTTP_DEFAULT_PORT = 80
val HTTPS_DEFAULT_PORT = 443

# Example remote endpoints (commented out – strings not supported)
# val API_SERVER = "api.example.com"
# val API_PATH   = "/v1/data"
# val API_KEY    = "abc123"

# Timeout values (in seconds)
val HTTP_CONNECT_TIMEOUT = 10
val HTTP_READ_TIMEOUT    = 30

# Simulated response codes (can be used in if statements)
# For instance, you could have:
# if USE_WEB_FEATURES
#     val server_response = HTTP_OK
# else
#     val server_response = HTTP_NOT_FOUND
# end