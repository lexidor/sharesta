<?hh // partial
namespace Usox\Sharesta;

class ApiFactoryTest extends \PHPUnit_Framework_TestCase {

	private ?ApiFactory $api_factory;

	public function setUp(): void {
		$this->api_factory = new ApiFactory();
	}

	/**
	 * @dataProvider factoryMethodsDataProvider
	 */
	public function testCreationMethodsReturnInstances(
		string $method_name,
		classname<I> $expected_class_name,
		array<mixed> $parameter
	) {
		$this->assertInstanceOf(
			$expected_class_name,
			call_user_func_array([$this->api_factory, $method_name], $parameter)
		);
	}

	public function factoryMethodsDataProvider(): array<mixed> {
		return [
			['createRouter', Router::class, [ImmMap{}, ImmMap{}]],
			['createResponse', Response::class, [666, 'roedlbroem']],
		];
	}
}
