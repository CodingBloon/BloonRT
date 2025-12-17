#pragma once
#include <vector>
#include <string>

namespace Core {

	class SlangCompiler {
	public:
		SlangCompiler();
		~SlangCompiler();

		std::vector<char> compileShader(std::string& path);
	private:
		std::vector<char> readFile(std::string& path);
		std::vector<char> compile(std::vector<char>& code);
	};

}