# UX-04 — Model Workspace Final Validation

Status: **PASS — TECHNICAL V3 AND OWNER EXPERIENCE GATE APPROVED; UX-04 FROZEN** (2026-10-23).

## Baseline and authorization

The owner approved option B from [UX-04-REVIEW](UX_04_COMPLETION_REVIEW.md): final V3 and owner Experience Gate followed by a direct UX-04 freeze, without UX-04.3 or new functionality. Entry branch `master` was clean at `ab48d28`. The last executable change was `0d10290` ([UX-04.2](UX_04_2_BROWSER_INSPECTOR_VALIDATION.md)); `b710f48` and `ab48d28` changed documentation only. [UX-04.1](UX_04_1_MODEL_SELECTION_VALIDATION.md) is represented by `b39646b` and `c186f92`. No reset, rebase or dependency update was made.

## Technical Gate V3

| Check | Result / evidence |
| --- | --- |
| Fresh CMake configure | **PASS** in ignored `out/ux-04-v3`; Visual Studio 17 2022 x64, MSVC 19.33, Windows 10 SDK 10.0.19041. Existing pinned FetchContent source directories were reused offline; no library version changed. |
| C++ standard | CMake targets require `cxx_std_20`; C++ extensions disabled. |
| Full Debug build | **PASS**: `micro_solidworks.exe` and `micro_solidworks_tests.exe` from the fresh configuration. `out/ux-04-v3-build.log` contains zero project compiler/linker warnings and zero errors. |
| Full CTest | **PASS: 952/952**, zero failures, 358.68 seconds, from this fresh configuration (`out/ux-04-v3-ctest.log`). |
| Runtime/owner | **PASS by owner observation** on the clean-build executable: Model workflow approved and normal File → Exit reported. No automated native-app control, screen capture or independently measured process exit code is claimed. |
| Repository hygiene | **PASS** — final `git diff --check`, status and tracked-artifact inspection; only documentation was changed for the freeze. Build/test products remain ignored under `out/`. |

The clean configure and complete test run belong to this V3; directed UX-04.1/04.2 results are supporting evidence, not substitutes for them. Build and test logs are ignored output, not committed source.

## Integrated UX-04.1 and UX-04.2 contracts

UX-04.1 preserves exact logical Face/Edge targets while deriving coherent coplanar-face and collinear-edge highlight regions. Pocket-floor boundaries, depth-biased display, distinct Shaded/Shaded With Edges/Wireframe behavior, cyan hover and Orange/Blue selected accents passed directed picker/framebuffer tests and the owner gate. UX-04.2 derives the ordered Browser from FeatureModel, presents absorbed Sketches once, keeps names and icons separate, and groups Inspector identity, properties, relationships, applicable actions and diagnostics. Confirm/Cancel, rebuild reconciliation, Delete/Undo, Save/Open and SET-01 isolation passed directed tests and the owner gate. The earlier owner-found icon/name overlap was fixed and confirmed; this freeze has no new evidence reopening it.

## Architecture and reference audit

- `FeatureModel`, Geometry and Topology remain CAD authority. `FeaturePresentation`, SolidDisplay and WorkspaceViewport derive display/picking geometry; `EditorContext` owns transient tool and logical selection state. Application-owned Preferences remain separate from document dirty/Undo, and `.microsw` persistence is unchanged.
- `git diff --name-only 0d10290 HEAD -- CMakeLists.txt src tests third_party` is empty at entry: neither executable source nor dependency configuration changed after the approved UX-04.2 executable commit. CMake target links preserve the intended direction; no kernel or UI framework substitution was found.
- All **71/71** tracked ADRs are ACCEPTED. The four checked-in canonical PNG SHA-256 values match the [visual registry](uiux_v03_pkg/VISUAL_REFERENCE_REGISTRY_V0_3.md). [UX-REF-01-MODEL](uiux_v03_pkg/images/UX-REF-01-MODEL.png) is the principal composition comparator; [UX-REF-02-SKETCH](uiux_v03_pkg/images/UX-REF-02-SKETCH.png) checks shared shell continuity. [UX-REF-03-MANUFACTURE](uiux_v03_pkg/images/UX-REF-03-MANUFACTURE.png) and [UX-REF-04-EXPORT](uiux_v03_pkg/images/UX-REF-04-EXPORT.png) are future consistency references, not commands or functionality authorized by UX-04.

## Final owner Experience Gate

**Owner decision: APPROVED.** The owner used `out/ux-04-v3/Debug/micro_solidworks.exe`, reported completing the other applicable steps of the 30-step Model workflow, accepted the second-Cut limitation as nonblocking, and confirmed closure by **File → Exit**. The reviewed steps cover shell legibility/overlap and viewport; Browser Feature, face and pocket-floor edge selection in all three display modes; edit draft Cancel/Confirm, Undo/Redo and Rebuild; absorbed Sketch Edit/Exit; Save/Close/Open; Orange/Blue/Orange preferences without dirty/Undo effects. The `ux-01-gate.microsw` fixture has a pocket but no verified round hole in this final run; the requested hole was not added by the unsupported second Cut. A final fixture containing both hole and pocket was **NOT VERIFIED** in this gate and was accepted by the owner as nonblocking for UX-04. The owner did not supply screen resolution or scaling; manual 125%/150% remains **NOT VERIFIED**. No automated visual capture is claimed.

The first final-gate attempt used `out/ux-01-gate.microsw`, a part whose existing pocket is already a Cut Extrude. The owner tried a *new* Cut Extrude on the pocket floor and received `This Cut Extrude target cannot be reconstructed as a supported axial extrusion`. [`FeatureModel::rebuild`](../src/features/FeatureModel.cpp) requires the target result feature to be an additive Extrude before calling the supported axial subtraction. The same second-Cut limitation is explicitly recorded in the [v1.2 geometry preflight](PERSONAL_SOLIDWORKS_V1_2_GEOMETRY_PREFLIGHT.md) and [portable guide](PERSONAL_SOLIDWORKS_V1_2_PORTABLE_GUIDE.md); it predates UX-04 and is not evidence of a new rendering/Browser/Inspector regression. The suggested test route was misleading because it invited adding a hole by a second Cut to this already-cut fixture. A supported one-Cut hole-and-pocket sequence puts the circular hole in the *base Sketch* before additive Extrude, then applies one Cut for the pocket. The owner explicitly classified the extra Cut as nonblocking for this UX gate. No kernel extension was made or authorized by this freeze.

The current implementation follows the reference's left Browser, dominant Graphics Area, right Inspector, contextual command region, document identity and status layout. The owner's final observation, not an automated pixel comparison, determines visual acceptability. The canonical images illustrate additional future commands that are not present in the current product.

## Accepted debt and evidence limits

- **P2:** Header/mode/ribbon visual hierarchy, viewport framing/staging, HUD/orientation cues and additional finish.
- **P3:** search on larger feature trees, interactive View Cube and advanced geometric manipulation. No inert placeholders or new CAD identity are introduced.
- **HiDPI:** manual 125%/150% Windows scaling remains **NOT VERIFIED**; the owner explicitly reported it was not tested. This absence is nonblocking under the approved freeze scope. Directed layout frames at 1366×768 and 1920×1080 do not constitute actual HiDPI proof; final manual resolution/scale were not supplied.
- **Fixture boundary:** a hole-and-pocket part was not verified in the final UX-04 owner run; the already-pocketed fixture cannot gain a hole via a second Cut under the frozen v1.2 kernel. The owner accepted this as nonblocking for the Model Workspace refinement gate. The separate v1.2 bounded N2 acceptance and its exceptions are not revised here.
- Failed/blocked Inspector screenshots and same-fixture visual measurements were not archived. Their absence is an evidence limit, not a demonstrated runtime defect.

## Freeze decision and handoff

**PASS / FROZEN.** Clean V3 passed at executable source HEAD `0d10290`, the owner approved the final Experience Gate on that build and closed normally, the known second-Cut limit was accepted as nonblocking, and no architecture or executable changes followed. UX-04.1 and UX-04.2 remain COMPLETE. This freeze changes documentation only and uses V0; V3 is not repeated for the documentary commit. The authorized annotated tag is `ux-04-model-workspace-refinement`. No executable regression correction or additional UX-04.3 slice was needed.

UX-03 Export UX is the **next candidate, NOT STARTED** and requires separate authorization. The frozen v1.2.0 STL contract, P0/SPEC-01/SET-01/UX-01/UX-01.R1/UX-02 baselines and 71 accepted ADRs are unchanged. No push is performed; the owner publishes commits and tags.
