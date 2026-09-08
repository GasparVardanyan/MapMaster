# pragma once

# include <condition_variable>
# include <functional>
# include <memory>
# include <mutex>
# include <tuple>
# include <type_traits>
# include <vector>



namespace MapMaster::Tanki::Utils {

namespace ParallelTaskRunner_detail {

template <typename ...>
struct TaskCallbackInput;

template <typename Output, typename ... InputArgTypes>
struct TaskCallbackInput <Output, std::tuple <InputArgTypes ...>> {
	using type = std::tuple <InputArgTypes ..., std::shared_ptr <Output>>;
};

template <typename...>
struct TaskProcessor;

template <typename Producer, typename Output, typename ... InputArgTypes>
struct TaskProcessor <Producer, std::tuple <InputArgTypes ...>, Output> {
	using type = Output (Producer::*) (const InputArgTypes & ...) const;
};

} // namespace ParallelTaskRunner_detail



template <class Producer, typename OutputType, typename ... InputArgTypes>
class ParallelTask {
public:
	using Input = std::tuple <InputArgTypes ...>;
	using Output = OutputType;
	using CallbackInput = ParallelTaskRunner_detail::TaskCallbackInput <Output, Input>::type;

	using Callback = std::function <void (std::vector <CallbackInput> &&)>;
	using Processor = ParallelTaskRunner_detail::TaskProcessor <Producer, Input, Output>::type;

	ParallelTask (const Producer & producer, Processor processor);

	void reset ();

	template <
		bool Collect = true,
		bool PassToCallback = true,
		auto ResultMutator = std::identity {}
	>
	std::enable_if_t <
		std::is_invocable_v <decltype (ResultMutator), std::shared_ptr <Output>>,
		std::conditional_t <Collect, std::vector <
			std::remove_cvref_t <std::invoke_result_t <decltype (ResultMutator), std::shared_ptr <Output>>>
		>, void>
	> run (
		const std::vector <Input> & inputVector
	);

	void listen (Callback callback);

private:
	std::mutex m_readyMutex;
	std::condition_variable m_readyNotifier;
	std::vector <CallbackInput> m_callbackInput;
	bool m_finished = false;
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
	const Producer & m_producer;
	Processor m_processor;
};

}  // namespace MapMaster::Tanki::Utils
