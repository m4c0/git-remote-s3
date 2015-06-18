# git-remote-s3

A simple way to use Amazon Web Services's S3 as a remote for GIT

In theory, you just need to clone this repo, build the Visual Studio project and put the git-remote-s3.exe into your git's "bin" folder. Then, you can add "s3://[bucket]/[folder]" as a remote. Also, add "aws.secretkey" and "aws.accesskey" to your configs (either repo or global).

This is a small toy I made in my free time, and smells like a PoC. It is partially working (can push blobs, trees, commits and refs), but there's a lot to do, yet (like pulling back :).

BTW, there's a big catch: it's freaking slow. Maybe I will try to make "packs", like git's GC, to speed things up (and save some PUTs to S3) - I gladly accept ideas and examples about that.

Anyway, play with it as-is, and remember it's published in GPLv2.
