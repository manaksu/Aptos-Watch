# Aptos Watch — Pebble Time Steel (basalt)

Minimalist watchface: `<` symbol in Aptos Display Bold 48, time in Aptos Display 36, date in Aptos Display 22.

---

## Layout (144 × 168 px)

```
┌──────────────────┐
│                  │
│        <         │  ← Aptos Display Bold, 48pt, white
│                  │
│      14:32       │  ← Aptos Display, 36pt, white
│   Wed, 13 May    │  ← Aptos Display, 22pt, light grey
└──────────────────┘
```

---

## CloudPebble Setup

### Resources to upload

Upload both TTFs under **Resources → Add Resource → Font**:

| File | CloudPebble path |
|------|-----------------|
| `Aptos-Display-Bold.ttf` | `fonts/Aptos-Display-Bold.ttf` |
| `Aptos-Display.ttf` | `fonts/Aptos-Display.ttf` |

### package.json font registrations

Three slices (add directly in CloudPebble's package editor or paste the JSON):

| Resource ID | File | Size | characterRegex |
|---|---|---|---|
| `APTOS_BOLD_48` | Aptos-Display-Bold.ttf | 48 | `[<]` |
| `APTOS_36` | Aptos-Display.ttf | 36 | `[0-9:]` |
| `APTOS_22` | Aptos-Display.ttf | 22 | `[A-Za-z0-9 ,.]` |

### Source file

Add `src/c/main.c` as the sole C source.

---

## Tuning the symbol position

Pebble's font renderer inserts top-padding that varies by font. After your
first build:

- If `<` appears **too low** → decrease `s_symbol_layer` y (currently `22`)
- If `<` appears **clipped at bottom** → increase layer height (currently `62`)
- Adjust `s_time_layer` y (`94`) and `s_date_layer` y (`144`) accordingly

A common correction is –6 to –10 px on the symbol layer origin.
