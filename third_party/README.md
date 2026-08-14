# Third-party pins

Nothing here is copied from Bitcoin Toolkit 3.1.2.

| Path | Source | Commit / version | License |
|---|---|---|---|
| `picojson/picojson.h` | https://github.com/kazuho/picojson | `111c9be5188f7350c2eac9ddaedd8cca3d7bf394` | BSD-2-Clause |

Fetch again:

```sh
curl -fsSL -o third_party/picojson/picojson.h \
  https://raw.githubusercontent.com/kazuho/picojson/111c9be5188f7350c2eac9ddaedd8cca3d7bf394/picojson.h
```

`libsecp256k1` and LevelDB are distro packages, not vendored. See the top-level README.
