CXX = g++
CXXFLAGS = -g -O3 -std=c++17 -Wall -ftree-vectorize -I ./Include
LDFLAGS = -pthread

.PHONY: clean
OBJS = odd-even-seq odd-even-par-static odd-even-par-dyn odd-even-ff


%-p: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DPRINT $< -o $@

%-s: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DSTATS $< -o $@

# Utils
clean:
	rm -f *-p *-s
	rm -f $(OBJS)