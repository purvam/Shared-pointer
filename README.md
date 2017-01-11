# Shared-pointer

Implemented thread-safe, exception-safe, reference counting shared pointer, simulating std::shared_ptr in C++.

Designed to keenly avoid object-slicing for non-virtual destructors, i. e. when the smart pointer determines that the object should be deleted, it must delete the object via a pointer to the original type, even if the template argument of the final smart pointer is of a base type.
