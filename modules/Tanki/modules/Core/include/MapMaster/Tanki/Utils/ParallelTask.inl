# include "MapMaster/Tanki/Utils/ParallelTask.hpp"

# include <cstddef>
# include <execution>
# include <memory>
# include <mutex>
# include <type_traits>
# include <utility>
# include <vector>

namespace MapMaster::Tanki::Utils {

namespace ParallelTaskRunner_detail {
template <
	typename Input,
	typename = std::make_index_sequence <std::tuple_size_v <Input>>
> struct TaskInputProcessorHelper;

template <typename Input, std::size_t ... InputIndices>
struct TaskInputProcessorHelper <
	Input,
	std::index_sequence <InputIndices ...>
> {
	template <typename Producer, typename Processor, typename Input1>
	static inline decltype (auto) Process (
		const Producer & producer,
		Processor processor,
		Input1 && taskInput
	) {
		return (producer.*processor) (
			std::get <InputIndices> (std::forward <Input1> (taskInput)) ...
		);
	}

	template <
		typename CallbackInput,
		typename Input1,
		typename Output
	>
	static inline decltype (auto) InsertOutput (
		std::vector <CallbackInput> & callbackInput,
		Input1 && taskInput,
		std::shared_ptr <Output> taskOutput
	) {
		callbackInput.emplace_back (
			std::get <InputIndices> (std::forward <Input1> (taskInput)) ...,
			std::move (taskOutput)
		);
	}
};

}  // namespace ParallelTaskRunner_detail



template <class Producer, typename OutputType, typename ... InputArgTypes>
ParallelTask <Producer, OutputType, InputArgTypes ...>::ParallelTask (
	const Producer & producer,
	Processor processor
) : m_producer (producer), m_processor (processor) {}

template <class Producer, typename OutputType, typename ... InputArgTypes>
void ParallelTask <Producer, OutputType, InputArgTypes ...>::reset () {
	m_finished = false;
	m_callbackInput.clear ();
}

template <class Producer, typename OutputType, typename ... InputArgTypes>
template <bool Collect, bool PassToCallback>
std::conditional_t <Collect, std::vector <
	std::shared_ptr <typename ParallelTask <Producer, OutputType, InputArgTypes ...>::Output>
>, void> ParallelTask <Producer, OutputType, InputArgTypes ...>::run (
	const std::vector <ParallelTask <Producer, OutputType, InputArgTypes ...>::Input> & inputVector
) {
	reset ();

	auto process = [this] (
		const Input & input
	) -> std::shared_ptr <Output> {
		std::shared_ptr <Output> result = std::make_shared <Output> (
			ParallelTaskRunner_detail::TaskInputProcessorHelper <Input>::Process (
				m_producer,
				m_processor,
				input
			)
		);

		if constexpr (true == PassToCallback) {
			{
				std::scoped_lock <std::mutex> lock (m_readyMutex);
				ParallelTaskRunner_detail::TaskInputProcessorHelper <Input>::InsertOutput (
					m_callbackInput,
					input,
					result
				);
			}

			m_readyNotifier.notify_one ();
		}

		return result;
	};

	std::vector <std::shared_ptr <Output>> output;

	if constexpr (true == Collect) {
		output.resize (inputVector.size ());

		std::transform (
			std::execution::par_unseq,
			inputVector.cbegin (),
			inputVector.cend (),
			output.begin (),
			process
		);
	}
	else {
		std::for_each (
			std::execution::par_unseq,
			inputVector.cbegin (),
			inputVector.cend (),
			process
		);
	}

	if constexpr (true == PassToCallback) {
		{
			std::scoped_lock <std::mutex> lock (m_readyMutex);
			m_finished = true;
		}

		m_readyNotifier.notify_one ();
	}

	if constexpr (true == Collect) {
		return output;
	}
}

template <class Producer, typename OutputType, typename ... InputArgTypes>
void ParallelTask <Producer, OutputType, InputArgTypes ...>::listen (
	ParallelTask <Producer, OutputType, InputArgTypes ...>::Callback callback
) {
	while (true) {
		std::vector <CallbackInput> resultToProcess;

		bool finished = false;

		{
			std::unique_lock <std::mutex> lock (m_readyMutex);
			m_readyNotifier.wait (lock, [this] () -> bool {
				return false == m_callbackInput.empty () || true == m_finished;
			});

			resultToProcess = std::move (m_callbackInput);
			m_callbackInput.clear ();
			finished = m_finished;
		}

		callback (std::move (resultToProcess));

		if (true == finished) {
			break;
		}
	}
}

} // namespace MapMaster::Tanki::Utils
