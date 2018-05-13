# CPM PRAC4

## Testing local environment:
```sh
$ sh ./run.sh
```

## Zoo commands
```sh
$ export URV_USER=<USER>
```

Create tunel:
```sh
$ ssh -NL 8022:zoo:22 $URV_USER@portal1-deim.urv.cat
```

Login to zoo:
```sh
$ ssh -X -p 8022 $URV_USER@localhost
```

Copy files to zoo:
```sh
$ scp -P 8022 fitxer $URV_USER@localhost:.
```
