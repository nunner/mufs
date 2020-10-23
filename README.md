# About
mufs is a FUSE driver which allows you to mount your music collection and browse it by its tags.

### Features
- change a files' tags by moving/renaming it in mufs.
- browse your music collection without having to order the physical files.

### Formatting options
|    | Formatting options |
|----|--------------------|
| %a | artist             |
| %f | album              |
| %t | title              |
| %n | track number       |
| %g |  genre             |

Specify using a format using the --format flag. Example: 
```
mufs --format="%a/%f/%n - %t" root mount
```

# Dependencies
mufs depends on
- taglib
- fuse3

# Installation
```
make
```
and 
```
sudo make install
```
