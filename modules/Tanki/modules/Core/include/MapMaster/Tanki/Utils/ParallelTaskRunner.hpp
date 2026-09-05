# pragma once

# include <condition_variable>
# include <functional>
# include <memory>
# include <mutex>
# include <tuple>
# include <vector>



namespace MapMaster::Tanki::Utils {

namespace ParallelTaskRunner_detail {

template <typename ...>
struct TaskCallbackInput;

template <typename Output, typename ... InputArgs>
struct TaskCallbackInput <Output, std::tuple <InputArgs ...>> {
	using type = std::tuple <InputArgs ..., std::shared_ptr <Output>>;
};

template <typename...>
struct TaskProcessor;

template <typename Producer, typename Output, typename ... InputArgs>
struct TaskProcessor <Producer, std::tuple <InputArgs ...>, Output> {
	using type = Output (Producer::*) (const InputArgs & ...) const;
};

} // namespace ParallelTaskRunner_detail



template <class Producer, class ParallelTask>
class ParallelTaskRunner {
public:
	using Task = ParallelTask;
	using Input = Task::Input;
	using Output = Task::Output;
	using CallbackInput = ParallelTaskRunner_detail::TaskCallbackInput <Output, Input>::type;

	using Callback = std::function <void (std::vector <CallbackInput> &&)>;
	using Processor = ParallelTaskRunner_detail::TaskProcessor <Producer, Input, Output>::type;

	ParallelTaskRunner (const Producer & producer, Processor processor);

	void reset ();

	std::vector <std::shared_ptr <Output>> run (
		const std::vector <Input> & input
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

template <typename OutputType, typename ... InputArgs>
struct ParallelTask {
	using Input = std::tuple <InputArgs ...>;
	using Output = OutputType;

	ParallelTask ();
};

}  // namespace MapMaster::Tanki::Utils
