<?hh
namespace Usox\Sharesta;

interface RequestInterface {

	public function getRoute(): string;

	public function getHttpMethod(): string;

	public function setRouteParameters(Map<string,string> $parameters): void;

	public function getRouteParameters(): Map<string,string>;

	public function getUriValues(): Map<string,string>;
	
	public function getBodyAsJson(): string;

	public function getBody(): Map<string,string>;
}