# derTimeGeraet
Time Machine like backup tool based on Borg. A rsync based version can be found in the rsync branch.
A new backup repo must be initialized manually:

```bash
$ borg init --encryption=repokey /path/to/repo
```

The tool is still in development and therefore it can not be considered as stable yet.

![Screenshot](https://raw.githubusercontent.com/myzinsky/derTimeGeraet/master/img/screenshot.png "Screenshot")
