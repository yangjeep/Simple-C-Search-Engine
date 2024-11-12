# C Search Engine Project

A simple search engine implementation in C that demonstrates indexing and document search capabilities.

## Backend Architecture

The search engine's backend is powered by a generalized trie (GTrie) data structure, which efficiently stores and retrieves indexed terms. The GTrie implementation provides:

- Fast prefix-based searching capabilities
- Memory-efficient storage of terms through shared prefixes
- Full UTF-8 support for international character sets
- Integrated posting lists for document references
- Persistent storage using LMDB (Lightning Memory-Mapped Database)

Each node in the GTrie can store a UTF-8 codepoint and maintains:
- Links to child nodes (supporting full Unicode range)
- A posting list containing document references
- A flag indicating if the node represents the end of a word

The GTrie is complemented by LMDB for persistent storage, allowing the search index to be saved and loaded between sessions efficiently.


## Prerequisites

- CMake (version 3.14 or higher)
- C compiler (gcc/clang)
- Git

## Building the Project

1. Clone the repository:


```bash
git clone https://github.com/yourusername/search-engine.git
```

2. Navigate to the project directory:

```bash
cd search-engine
```

3. Build the project using CMake:

```bash
mkdir build
cd build
cmake ..
make
```
