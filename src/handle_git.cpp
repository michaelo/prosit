#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>

#include "app.h"

#include "mlib/defer.imp.h"

Handler_Status handle_git(Context *c, Manifest_Entry *e)
{
    (void)c;

    // Att! This destructs the Manifest_Entry <- thus if this for some reason is processed multiple times the behaviour is undefined
    char cmd[1024];
    char scrap[1024];
    // char *dest;
    char *ref = nullptr;

    // Extract ref/branch/tag from source-part and update afterwards
    char *tok = strchr(e->src, '#');
    if(tok != nullptr) {
        *tok = '\0';
        ref = tok+1;
    }

    // If dest does not exist: clone
    if(!std::filesystem::exists(e->dst)) {
        snprintf((char *)cmd, sizeof(cmd), "git clone %s %s", e->src, e->dst);
        printf("About to execute: %s\n", cmd);
        if (std::system(cmd) != 0)
        {
            printf("ERROR: Got error executing git clone. See message(s) above.\n");
            return Handler_Status::Error;
        }
    } else {
        // If dest exists and is a git-repo: pull, checkout
        // Check if an actual git-repo or a collision
        // TBD: Check if remote is the same as specified in src also?
        // If dest exists and is not a git-repo: abort
        auto cwd = std::filesystem::current_path();
        defer(std::filesystem::current_path(cwd));

        snprintf((char*)scrap, sizeof(scrap), "%s/.git", e->dst);
        if(std::filesystem::exists(scrap)) {

            printf("Dest exists... Let's pull instead\n");
            std::filesystem::current_path(e->dst);

            snprintf((char *)cmd, sizeof(cmd), "git pull");
            printf("About to execute: %s (cwd: %s)\n", cmd, std::filesystem::current_path().c_str());
            if (std::system(cmd) != 0)
            {
                printf("ERROR: Got error executing git pull. See message(s) above.\n");
                return Handler_Status::Error;
            }
        } else {
            printf("ERROR: desitination alread exists, but is not a GIT-repo. Aborting.\n");
            return Handler_Status::Error;
        }
    }

    // TBD: What if ref at some point was set, but now is not? Always require a ref to avoid ambiguity? The user can always just use a branch.
    if(ref != nullptr) {
        // Checkout
        auto cwd = std::filesystem::current_path();
        defer(std::filesystem::current_path(cwd));


        std::filesystem::current_path(e->dst);
        snprintf((char *)cmd, sizeof(cmd), "git checkout %s", ref);
        printf("About to execute: %s (cwd: %s)\n", cmd, std::filesystem::current_path().c_str());
        if (std::system(cmd) != 0)
        {
            printf("ERROR: Got error executing git checkout. See message(s) above.\n");
        }
    }

    return Handler_Status::OK;
}
