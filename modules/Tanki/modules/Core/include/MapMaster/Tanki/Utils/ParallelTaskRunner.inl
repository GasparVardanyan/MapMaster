# include "MapMaster/Tanki/Utils/ParallelTaskRunner.hpp"

# include <cstddef>
# include <execution>
# include <memory>
# include <mutex>
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



template <class Producer, class ParallelTask>
ParallelTaskRunner <Producer, ParallelTask>::ParallelTaskRunner (
	const Producer & producer,
	Processor processor
) : m_producer (producer), m_processor (processor) {}

template <class Producer, class ParallelTask>
void ParallelTaskRunner <Producer, ParallelTask>::reset () {
	m_finished = false;
	m_callbackInput.clear ();
}

template <class Producer, class ParallelTask>
std::vector <
	std::shared_ptr <typename ParallelTaskRunner <Producer, ParallelTask>::Output>
> ParallelTaskRunner <Producer, ParallelTask>::run (
	const std::vector <ParallelTaskRunner <Producer, ParallelTask>::Input> & input
) {
	reset ();

	std::vector <std::shared_ptr <Output>> output;
	output.resize (input.size ());

	std::transform (
		std::execution::par_unseq,
		input.cbegin (),
		input.cend (),
		output.begin (),
		[this] (
			const Input & descriptor
		) -> std::shared_ptr <Output> {
			std::shared_ptr <Output> result = std::make_shared <Output> (
				ParallelTaskRunner_detail::TaskInputProcessorHelper <Input>::Process (
					m_producer,
					m_processor,
					descriptor
				)
			);

			{
				std::scoped_lock <std::mutex> lock (m_readyMutex);
				ParallelTaskRunner_detail::TaskInputProcessorHelper <Input>::InsertOutput (
					m_callbackInput,
					descriptor,
					result
				);
			}

			m_readyNotifier.notify_one ();

			return std::move (result);
		}
	);

	{
		std::scoped_lock <std::mutex> lock (m_readyMutex);
		m_finished = true;
	}

	m_readyNotifier.notify_one ();

	return output;
}

template <class Producer, class ParallelTask>
void ParallelTaskRunner <Producer, ParallelTask>::listen (
	ParallelTaskRunner <Producer, ParallelTask>::Callback callback
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
