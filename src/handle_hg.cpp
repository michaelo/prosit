#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <cassert>

#include "app.h"

#include "mlib/defer.imp.h"

// Main entry point for updating a hg-entry
// 
// * Extracts the (for now optional) reference-part from src
// * Checks if dst already exists:
//   * if not exists: clone
//   * if exists and contains is a hg-repo: pull
//   * if exists and is not a hg-repo: abort
// * if ref specified: checkout accordingly
// 
App_Status_Code handle_hg(Context *c, Manifest_Entry *e)
{
    // Att! This modifies the Manifest_Entry <- thus if this for some reason is processed multiple times the behaviour is undefined
    char scrap[1024];
    // char *dest;
    char *ref = nullptr;

    // Extract ref/branch/tag from source-part and update afterwards
    char *tok = strchr(e->src, '#');
    if(tok != nullptr) {
        *tok = '\0';
        ref = tok+1;
    }
    // TODO: _Require_ a ref?

    // If dest does not exist: clone
    if(!std::filesystem::exists(e->dst)) {
        assert(snprintf((char *)scrap, sizeof(scrap), "hg clone %s %s %s", e->src, e->dst, c->silent ? "-q" : "") > 0);
        c->info("About to execute: %s\n", scrap);
        if (std::system(scrap) != 0)
        {
            c->error("Got error executing hg clone. See message(s) above.\n");
            return App_Status_Code::Error;
        }
    } else {
        auto cwd = std::filesystem::current_path();
        defer(std::filesystem::current_path(cwd));

        assert(snprintf((char*)scrap, sizeof(scrap), "%s/.hg", e->dst) > 0);
        // If dest exists and is a hg-repo: pull, checkout
        // Check if an actual hg-repo or a collision
        if(std::filesystem::exists(scrap)) {
            // TBD: Check if remote is the same as specified in src also?
            c->debug("Dest exists... Let's pull instead\n");
            std::filesystem::current_path(e->dst);

            assert(snprintf((char *)scrap, sizeof(scrap), "hg pull %s", c->silent ? "-q" : "") > 0);
            c->debug("About to execute: %s (cwd: %s)\n", scrap, std::filesystem::current_path().u8string().c_str());
            if (std::system(scrap) != 0)
            {
                c->error("Got error executing hg pull. See message(s) above.\n");
                return App_Status_Code::Error;
            }
        } else {
            // If dest exists and is not a hg-repo: abort
            c->error("Destination already exists, but is not a hg-repo. Aborting.\n");
            return App_Status_Code::Error;
        }
    }

    // TBD: What if ref at some point was set, but now is not? Always require a ref to avoid ambiguity? The user can always just use a branch.
    if(ref != nullptr) {
        // Checkout
        auto cwd = std::filesystem::current_path();
        defer(std::filesystem::current_path(cwd));

        std::filesystem::current_path(e->dst);
        assert(snprintf((char *)scrap, sizeof(scrap), "hg checkout %s %s", ref, c->silent ? "-q" : "") > 0);
        c->debug("About to execute: %s (cwd: %s)\n", scrap, std::filesystem::current_path().u8string().c_str());
        if (std::system(scrap) != 0)
        {
            c->error("Got error executing hg checkout. See message(s) above.\n");
        }
    }

    return App_Status_Code::Ok;
}
