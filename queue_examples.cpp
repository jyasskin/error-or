// Example API usage for various error handling patterns.

#include <vector>

// Imagine a hypothetical "freelist" allocator.
template <typename T> class my_freelist;

// This allocator is non-propagating by default -- it is a C++98 container.
namespace std {
template <> allocator_traits<my_freelist> {
  // ...
};

// And a container type using this allocator.
class wombat;
typedef std::vector<wombat, my_freelist<wombat>> wombat_vector;

// Some routine for building these wombat vectors based on user input.
wombat_vector build_wombats(my_freelist<wombat> &freelist);

// Some routine for processing wombats and upload them to a server.
void process_wombats(wombat_vector&& wombats);

void producer(std::shared_queue_back<wombat_vector>& queue,
              my_freelist<wombat> &freelist) {
  while (/* read data from network, whatever */) {
    queue.push_back(build_wombats(freelist));
  }
}

void consumer1(std::shared_queue_back<wombat_vector>& queue) {
  try {
    for (;;) {
      wombat_vector wombats{queue.try_pop()};

      // Whatever processing needs to occur...
      process_wombats(std::move(wombats));
    }
  } catch (const queue_error& error) {
    // ...
  }
}

void consumer2(std::shared_queue_back<wombat_vector>& queue) {
  wombat_vector wombats;
  for (;;) {
    if (std::error_code e = queue.try_pop(wombats)) {
      // Handle error...
      return;
    }

    // Whatever processing needs to occur...
    process_wombats(std::move(wombats));
  }
}

void consumer3(std::shared_queue_back<wombat_vector>& queue) {
  for (;;) {
    std::error_or<wombat_vector> wombats = queue.try_pop(wombats);
    if (std::error_code e = wombats.error()) {
      // Handle error...
      return;
    }

    // Whatever processing needs to occur...
    process_wombats(wombats.get());
  }
}

void consumer4(std::shared_queue_back<wombat_vector>& queue) {
  try {
    for (;;) {
      std::error_or<wombat_vector> wombats{queue.try_pop()};

      // Whatever processing needs to occur...
      process_wombats(wombats.get()); // This is safe, will throw if wrong.
    }
  } catch (const std::system_error& error) {
    // ...
  }
}

