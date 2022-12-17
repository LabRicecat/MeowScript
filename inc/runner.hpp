#ifndef MEOWSCRIPT_IG_RUNNER_HPP
#define MEOWSCRIPT_IG_RUNNER_HPP

#include "reader.hpp"
#include "scopes.hpp"
#include "commands.hpp"
#include "variables.hpp"
#include "commands.hpp"
#include "global.hpp"
#include "errors.hpp"

#include <filesystem>

MEOWSCRIPT_HEADER_BEGIN

Variable run_file(std::string file, bool new_scope = true, bool save_scope = false, int load_idx = -1, std::map<std::string,Variable> external_vars = {}, fs::path from = "", bool pass_return_down = false, bool same_scope = false);
Variable run_text(std::string text, bool new_scope = true, bool save_scope = false, int load_idx = -1, std::map<std::string,Variable> external_vars = {},fs::path from = "", bool pass_return_down = false, bool same_scope = false);
Variable run_lexed(lexed_tokens lexed, bool new_scope = true, bool save_scope = false, int load_idx = -1, std::map<std::string,Variable> external_vars = {},fs::path from = "", bool pass_return_down = false, bool same_scope = false);

MEOWSCRIPT_HEADER_END

#endif