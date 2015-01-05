#include "markov_chain.hpp"
#include "error.hpp"
#include <fstream>
#include <numeric>
#include <algorithm>
#include <iostream>

namespace unilingus {

BigramList make_bigram_list(const wstring& word) {
  if(word.size() < 2) {
    return BigramList();
  }

  BigramList bl(word.size() - 1);

  for (size_t i = 1; i < word.size(); ++i) {
    bl[i - 1] = { word.at(i - 1), word.at(i) };
  }

  return bl;
}
/*
void MarkovChain::learn(const wstring& file, CharAcceptor ca) {
  std::wifstream is(wstring(file));
  learn(is,ca);
}*/

void MarkovChain::learn(std::wistream& is, CharAcceptor ca) {
  std::cerr << "training markov chain" << std::endl;

  wstring line;

  size_t cnt = 0;
  while (std::getline(is, line)) {
    for (Bigram& b : make_bigram_list(line)) {
      ++((*this)[b.first][b.second]);
    }
    std::cerr << "\r" << ++cnt;
  }
  std::cerr << std::endl;

  //normalize
  double maxRelativeProbability = 0;
  for (auto& p1 : (*this)) {

    double sum = 0;
    for (auto& p2 : p1.second) {
      CHECK(p2.second > 0);
      sum += p2.second;
    }

    for (auto& p2 : p1.second) {
        p2.second = p2.second/sum;
        maxRelativeProbability = std::max(maxRelativeProbability, p2.second);
    }
  }

  for (auto& p1 : (*this)) {
    for (auto& p2 : p1.second) {
      p2.second /= maxRelativeProbability;
      CHECK(p2.second >= 0.0 && p2.second <= 1.0);
    }
  }
}


void read_markov_chain(MarkovChain& mc, std::istream& is) {
#ifndef _NO_SERIALIZE
  boost::archive::binary_iarchive ia(is);
  ia >> mc;
#else
  CHECK(false);
#endif
}

void write_markov_chain(MarkovChain& mc, std::ostream& os) {
#ifndef _NO_SERIALIZE
  boost::archive::binary_oarchive oa(os);
  oa << mc;
#else
  CHECK(false);
#endif
}
} /* namespace eagereye */
