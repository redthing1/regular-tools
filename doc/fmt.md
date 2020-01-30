
# binary format

## sections
| Section | Size      | Description                             |
|---------|-----------|-----------------------------------------|
| Header  | 8         | Magic header, entry, and section sizes. |
| Data    | `data_sz` | Misc data.                              |
| Code    | `code_sz` | REGULAR_ad instructions.                |

## header

- `"ad"` [2] - magic constant
- `entry` [2] - entry point
- `code_sz` [2] - code size
- `data_sz` [2] - data size

## data

miscellaneous binary data.

word alignment is not compulsory but is strongly recommended.

## code

instructions.
