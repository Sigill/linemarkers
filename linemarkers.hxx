#ifndef LINEMARKERS_HXX
#define LINEMARKERS_HXX

#include <vector>
#include <deque>
#include <string>
#include <iosfwd>
#include <functional>
#include <memory>

class PreprocessedFile {
public:
  std::size_t included_at_line;
  std::string filename;
  std::vector<std::string> lines;
  std::vector<PreprocessedFile*> includes;
  std::size_t depth;
  std::size_t lines_count, cumulated_lines_count;
  std::size_t last_effective_line;

  explicit PreprocessedFile(std::size_t included_at_line, std::string filename);
};

class IncludeTree {
public:
  std::deque<PreprocessedFile> files;
  PreprocessedFile* root;

  IncludeTree();

  IncludeTree(const IncludeTree& other);
  IncludeTree(IncludeTree&& other);

  IncludeTree& operator=(const IncludeTree& other);
  IncludeTree& operator=(IncludeTree&& other);

  static IncludeTree from_stream(std::istream& stream, const bool store_lines = true);

  template<typename InputIt>
  static IncludeTree from_lines(InputIt first, const InputIt last, const bool store_lines = true);

private:
  void relocate_files(const IncludeTree& other);
};

class LineMarkersParser_impl;

class LineMarkersParser {
private:
  std::unique_ptr<LineMarkersParser_impl> impl;

public:
  explicit LineMarkersParser(const bool store_lines);
  ~LineMarkersParser();

  void parseLine(const std::string& line);

  void finalize();

  IncludeTree& tree();
  const IncludeTree& tree() const;
};

template<typename InputIt>
IncludeTree IncludeTree::from_lines(InputIt first, const InputIt last, const bool store_lines) {
  LineMarkersParser parser(store_lines);

  for(; first != last; ++first) {
    parser.parseLine(*first);
  }

  parser.finalize();

  return std::move(parser.tree());
}

void preorder_walk(const IncludeTree& tree, std::function<void(const PreprocessedFile&)> cbk);

#endif // LINEMARKERS_HXX
