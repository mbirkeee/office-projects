To get pracman to run after a fresh checkout,
the libmysql.dll file mode must be made executable.

NOTE: I think that the above requirement is now
automatically  met since pracman is stored in git.


2019-02-04
----------
As os this writing I have pracman compiling from a fresh
git checkout.  

The pracman.ini file assumes that support directories
are located in X:\nobackup\pracman_data.  

To satisfy this requirement, connect to the samba server
as drive X, and in the nobackup directory, place a symlink
to the pracman_data directory in this repo.

NOTE: we want as must data as possible on the samba server,
NOT in Virtual Machine directories.

