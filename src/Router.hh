<?hh // strict
namespace Usox\Sharesta;

use namespace HH\Lib\Str;

final class Router implements RouterInterface {

	const HTTP_OK = 200;
	const HTTP_BAD_REQUEST = 400;
	const HTTP_NOT_FOUND = 404;
	const HTTP_INTERNAL_SERVER_ERROR = 500;

	private vec<string> $response_headers = vec[];

	public function __construct(
		private ApiFactoryInterface $api_factory,
		private RequestInterface $request
	): void {
	}

	private dict<string, (function (RequestInterface): \JsonSerializable)> $routes = dict[];

	public function register(string $route, (function (RequestInterface): \JsonSerializable) $callback, ?string $http_method = null): void {
		if ($http_method !== null) {
			$route = Str\format(
				'%s:%s',
				$http_method,
				$route
			);
		}

		$this->routes[$route] = $callback;
	}

	public function get(string $route, (function (RequestInterface): \JsonSerializable) $callback): void {
		$this->register($route, $callback, 'GET');
	}

	public function post(string $route, (function (RequestInterface): \JsonSerializable) $callback): void {
		$this->register($route, $callback, 'POST');
	}

	public function put(string $route, (function (RequestInterface): \JsonSerializable) $callback): void {
		$this->register($route, $callback, 'PUT');
	}

	public function delete(string $route, (function (RequestInterface): \JsonSerializable) $callback): void {
		$this->register($route, $callback, 'DELETE');
	}

	public function route(string $base_path): void {
		try {
			$response = \json_encode($this->matchRoute($base_path));
			$status_code = static::HTTP_OK;
		} catch (Exception\SharestaException $e) {
			$response = $e->getMessage();
			$status_code = $e->getHttpStatusCode();
		} catch (\Exception $e) {
			$response = 'Internal server error';
			$status_code = static::HTTP_INTERNAL_SERVER_ERROR;
		}

		$this->api_factory->createResponse(
			$status_code,
			$response
		)->send(
			$this->response_headers
		);
	}

	public function addResponseHeader(string $header): void {
		$this->response_headers[] = $header;
	}

	private function matchRoute(string $base_path): \JsonSerializable {
		$requested_route = $this->request->getRoute($base_path);
		$http_method = $this->request->getHttpMethod();
		$route_parameters = dict([]);

		foreach ($this->routes as $route => $callback) {
			// if the route also defines a HTTP method, use it as prefix for the lookup
			$route_method = \strstr($route, ':', true);
			$request_prepend = '';

			if ($route_method && \substr($route_method, -1) !== '/') {
				$request_prepend = Str\format('%s:', $http_method);
			}

			$route_pattern = \preg_replace(
				'%/:([^ /?]+)(\?)?%',
				'/\2(?P<\1>[^ /?]+)\2',
				$route
			);

			$uri_params = [];
			if (\preg_match_with_matches('%^'.$route_pattern.'$%', $request_prepend.$requested_route, inout $uri_params)) {

				foreach ($uri_params as $key => $value) {
					if (!\is_numeric($key)) {
						$route_parameters[$key] = $value;
					}
				}
				$this->request->setRouteParameters($route_parameters);

				return $callback($this->request);
			}
		}
		throw new Exception\NotFoundException('The requested resource was not found');
	}
}
