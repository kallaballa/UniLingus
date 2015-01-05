#include "error.hpp"
#include "markov_chain.hpp"
#include <fstream>
#include <locale>
#include <boost/locale.hpp>

#ifndef _NO_PROGRAM_OPTIONS
#include <boost/program_options.hpp>
#endif

#ifndef _NO_PROGRAM_OPTIONS
namespace po = boost::program_options;
#endif

namespace ul = unilingus;

using std::locale;
using std::ifstream;
using std::ofstream;
using std::string;
using std::wifstream;
using std::wofstream;
using std::wstring;
using std::cerr;
using std::endl;
using std::vector;

std::vector<wchar_t> VOWELS = {'a','e','i','o','u',L'ä',L'ö',L'ü', L'A', L'Э', L'У', L'О', L'Ы', L'Я', L'Е', L'Ё', L'Ю', L'И'};

bool isVowel(wchar_t c) {
  return find(VOWELS.begin(), VOWELS.end(), c) != VOWELS.end();
}

bool isConsonant(wchar_t c) {
    return !isVowel(c) && std::isalpha(c);
}
wchar_t findNext(const wchar_t& current, const wchar_t& next, ul::MarkovChain& mc) {
  ul::MarkovRow row = mc[current];
  std::vector<std::pair<wchar_t, double>> copy;

  for(auto& p : row) {
    copy.push_back(p);
  }

  std::sort(copy.begin(), copy.end(),
      [](std::pair<wchar_t, double> p1, std::pair<wchar_t, double> p2){ return p1.second > p2.second; }
  );

  double total = 0;
  for(auto& p : copy) {
    total += p.second;
  }

  std::default_random_engine rng(std::random_device{}());
  std::uniform_real_distribution<double> dist(0, total);

  double pie = dist(rng);
  double count = 0;
  bool nextIsVowel = isVowel(next);
  for(auto& p : copy) {
    count += p.second;
    if(count >= (pie / 2) && ((nextIsVowel && isVowel(p.first)) || (!nextIsVowel && isConsonant(p.first))))
      return p.first;
  }
  return next;
}

int main(int argc, char** argv) {
  //command line parsing
  string genMarkovFile;
  string runMarkovFile;
  string strLocale = "en_US.UTF-8";

  #ifndef _NO_PROGRAM_OPTIONS
  po::options_description genericDesc("Options");
  genericDesc.add_options()
      ("gen-markov,g", po::value<string>(&genMarkovFile),"Generate markov chain file. Read dictionary from stdin")
      ("run-markov,n", po::value<string>(&runMarkovFile),"Load the markov chain file and validate words from stdin using the markov chain")
      ("local,l", po::value<string>(&strLocale)->default_value(strLocale),"Set the locale")
      ("help,h", "Produce help message");

  po::options_description cmdline_options;
  cmdline_options.add(genericDesc);

  po::options_description visible;
  visible.add(genericDesc);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << "Usage: unilingus [options]\n";
    std::cerr << visible;
    return 0;
  }

#endif
  locale::global(locale(strLocale.c_str()));
  std::locale loc = boost::locale::generator().generate(strLocale);
  //std::wcin.imbue(loc);
  std::wcout.imbue(loc);

  ul::ErrorHandler::init(ul::default_error_delegate);

  srand(time(0));

  CHECK(runMarkovFile.size() || genMarkovFile.size());
  CHECK(! (runMarkovFile.size() && genMarkovFile.size()));

  ul::MarkovChain mc;

  if(runMarkovFile.size()) {
    std::cerr << "run markov chain" << std::endl;
    std::ifstream is(runMarkovFile);
    ul::read_markov_chain(mc, is);
    wstring line;

    while(std::getline(std::wcin, line)) {
      for(wchar_t& c : line) {
        c = std::tolower(c);
      }
      std::wstringstream ss(line);
      wstring token;

      while(ss) {
        ss >> token;
        if(token.empty())
          continue;

        wchar_t last = token.at(0);
        std::wcout << last;

        for(size_t i = 1; i < token.size(); ++i) {
          last = findNext(last, token.at(i), mc);
          std::wcout << last;
        }
        std::wcout << " ";
      }
      std::wcout << std::endl;
    }
  }

  if(genMarkovFile.size()) {
    ofstream os(genMarkovFile);
    mc.learn(std::wcin, [](const wchar_t& c){ return (bool)(c);});
    ul::write_markov_chain(mc, os);
  }
}
