#include "linemarkers.hxx"

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace bpo = boost::program_options;

namespace io {
class repeat {
public:
  const std::string& value;
  const size_t n;
  repeat(const std::string& value, size_t n)
    : value(value), n(n) {}
};

std::ostream& operator<<(std::ostream& os, const io::repeat& m) {
  for(size_t i = m.n; i > 0; --i)
    os << m.value;
  return os;
}

} // namespace io

void print(const IncludeTree& tree, const std::string& line_prefix = {}) {
  preorder_walk(tree, [&line_prefix](const PreprocessedFile& file){
    if (!line_prefix.empty())
      std::cout << line_prefix << " ";

    std::cout << io::repeat("| ", file.depth-1)
              << file.included_at_line << " "
              << file.filename << " (" << file.lines_count << " / " << file.cumulated_lines_count << ")" << std::endl;
  });
}

enum class filename_print_mode_t {
  none,
  head,
  line
};

void validate(boost::any& v,
              const std::vector<std::string>& values,
              filename_print_mode_t* /*target_type*/, int)
{
  // Make sure no previous assignment to 'v' was made.
  bpo::validators::check_first_occurrence(v);

  const std::string& s = bpo::validators::get_single_string(values);

  if (s == "none")
    v = filename_print_mode_t::none;
  else if (s == "head")
    v = filename_print_mode_t::head;
  else if (s == "line")
    v = filename_print_mode_t::line;
  else
    throw bpo::invalid_option_value(s);
}

int main(int argc, char** argv) {
  std::vector<std::string> input_files;

  bpo::options_description opts("Options");

  opts.add_options()
      ("help,h",
       "Produce help message.")
      ("file,f",
       bpo::value<std::vector<std::string>>(&input_files)->multitoken(),
       "Read from file instead of stdin.")
      ("filename", bpo::value<filename_print_mode_t>(),
       "Filename print mode:\n"
       "- none: Never print the filename. This is the default when reading from stdin or when processing a single file.\n"
       "- head: Print the filename before the tree This is the default when processing multiple files.\n"
       "- line: Prefix each result line with the prefix.")
      ;

  bpo::variables_map vm;

  try {
    bpo::store(bpo::command_line_parser(argc, argv).options(opts).run(), vm);

    if (vm.count("help")) {
      std::cout << opts;
      return 0;
    }

    bpo::notify(vm);
  } catch(bpo::error &err) {
    std::cerr << err.what() << std::endl;
    return -1;
  }

  filename_print_mode_t filename_print_mode = filename_print_mode_t::none;
  if (vm.count("filename") > 0)
    filename_print_mode = vm["filename"].as<filename_print_mode_t>();

  if (vm.count("file") > 0) {
    const auto files = vm["file"].as<std::vector<std::string>>();
    if (vm.count("filename") == 0 && files.size() > 1)
      filename_print_mode = filename_print_mode_t::head;

    for(const std::string& file : files) {
      if (!boost::filesystem::is_regular_file(file)) {
        std::cerr << file << " is not a regular file" << std::endl;
        continue;
      }

      std::ifstream in(file);
      if (in.bad()) {
        std::cerr << "Cannot read " << file << std::endl;
        continue;
      }

      if (filename_print_mode == filename_print_mode_t::head)
        std::cout << file << std::endl;

      const IncludeTree tree = IncludeTree::from_stream(in);
      print(tree, filename_print_mode == filename_print_mode_t::line ? file : "");
    }
  } else {
    const IncludeTree tree = IncludeTree::from_stream(std::cin);
    print(tree);
  }

  return 0;
}
