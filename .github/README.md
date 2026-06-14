# runit - a UNIX init scheme with service supervision

The release is available through <https://smarden.org/runit/>.

To build an experimental version from git, do
```
git clone -b next https://github.com/g-pape/runit
cd runit
package/compile
package/check
```
To install the experimental programs, as root do
```
install -m0755 command/* /bin/
mv /bin/runit* /sbin/
```
