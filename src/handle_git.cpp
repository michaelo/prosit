#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>

#include "app.h"

#include "mlib/defer.imp.h"

// TODO: output inside handlers should really only be for verbose as this gets messy when multithreading.

// Main entry point for updating a git-entry
// 
// * Extracts the (for now optional) reference-part from src
// * Checks if dst already exists:
//   * if not exists: clone
//   * if exists and contains is a git-repo: pull
//   * if exists and is not a git-repo: abort
// * if ref specified: checkout accordingly
// 
Handler_Status handle_git(Context *c, Manifest_Entry *e)
{
    (void)c;

    // Att! This destructs the Manifest_Entry <- thus if this for some reason is processed multiple times the behaviour is undefined
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
        snprintf((char *)scrap, sizeof(scrap), "git clone %s %s", e->src, e->dst);
        printf("About to execute: %s\n", scrap);
        if (std::system(scrap) != 0)
        {
            printf("ERROR: Got error executing git clone. See message(s) above.\n");
            return Handler_Status::Error;
        }
    } else {
        auto cwd = std::filesystem::current_path();
        defer(std::filesystem::current_path(cwd));

        snprintf((char*)scrap, sizeof(scrap), "%s/.git", e->dst);
        // If dest exists and is a git-repo: pull, checkout
        // Check if an actual git-repo or a collision
        if(std::filesystem::exists(scrap)) {
            // TBD: Check if remote is the same as specified in src also?
            c->debug("Dest exists... Let's pull instead\n");
            std::filesystem::current_path(e->dst);

            snprintf((char *)scrap, sizeof(scrap), "git pull");
            c->debug("About to execute: %s (cwd: %s)\n", scrap, std::filesystem::current_path().c_str());
            if (std::system(scrap) != 0)
            {
                c->error("Got error executing git pull. See message(s) above.\n");
                return Handler_Status::Error;
            }
        } else {
            // If dest exists and is not a git-repo: abort
            c->error("Destination already exists, but is not a git-repo. Aborting.\n");
            return Handler_Status::Error;
        }
    }

    // TBD: What if ref at some point was set, but now is not? Always require a ref to avoid ambiguity? The user can always just use a branch.
    if(ref != nullptr) {
        // Checkout
        auto cwd = std::filesystem::current_path();
        defer(std::filesystem::current_path(cwd));

        std::filesystem::current_path(e->dst);
        snprintf((char *)scrap, sizeof(scrap), "git checkout %s", ref);
        c->debug("About to execute: %s (cwd: %s)\n", scrap, std::filesystem::current_path().c_str());
        if (std::system(scrap) != 0)
        {
            printf("ERROR: Got error executing git checkout. See message(s) above.\n");
        }
    }

    return Handler_Status::OK;
}