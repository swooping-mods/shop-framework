# Shop Framework: behavior and implementation

Describes the retained payload **0.2.1**. The behavior prose describes the retained payload; current candidate source and metadata are identified below. Gameplay qualification is a separate requirement.


<!-- source-only-sync -->
## Current source candidate

Source version **0.2.1** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.2.1** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.

The authored behavior and lifecycle descriptions below are retained from the published payload. They are not new qualification evidence for the source candidate. The integration table below follows the current candidate metadata.

The exported test sources have deferred generated harness includes; their fixture closure is incomplete and they are not qualified executable tests. See `provenance.json` for the exact missing include identities.
<!-- /source-only-sync -->

## Behavior

Registers native mod offers into a separate Modded Shop tab opened with Ctrl+Shift+F2. Each offer supplies stable key, description/price, ownership/availability and purchase callback; vanilla shops retain vanilla rows. The framework owns one shortcut and native menu routing, while providers own eligibility, debit, grants and persistence. The optional Modded Shop entry uses the same service rather than another UI.

## State and lifecycle

Startup registry entries/callbacks remain resident; duplicate, invalid, late and overflowing offers are rejected. Native row/action routing revalidates the active shop context before invoking a provider. World/menu changes release mod-owned UI roots. It stores no universal ownership ledger and does not grant an item merely because its offer is registered.

## Integration and configuration

No additional feature-mod runtime dependency is declared. Included shared headers are source reuse, not extra DLL dependencies.

The [module metadata](../source/mods/shop-framework/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

| Service | ABI | Table size |
|---|---:|---|
| Provides `shop-framework.shop` | 1 | 56 bytes |

## Implementation sources

- [mod.c](../source/mods/shop-framework/mod.c)
- [registry.h](../source/mods/shop-framework/registry.h)
- [action.h](../source/mods/shop-framework/action.h)
- [sdk.h](../source/mods/shop-framework/sdk.h)
- [context.h](../source/mods/shop-framework/context.h)

## Verification boundary

Each provider must fail closed on authority/economy errors and decide its guest policy. Native UI/purchase integration is game-build-specific. A visible Buy button is not proof of transaction or save correctness.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
