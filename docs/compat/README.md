# Updating the compatibility chart

Edit `compat.json`, then run from the repo root:

```bash
python docs/compat/make_compat_svg.py docs/compat/compat.json docs/compatibility.svg
```

Statuses: `tested`, `builds`, `tight`, `no`, `unknown`. Only use `tested` after running the chip on real hardware with this fork.
