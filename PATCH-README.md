<h1 align="center" style="font-size: 80px;">🎮 🍴 RSX3MAC 🍴 🎮</h1>
## **Updated Technical Summary (Accurate to Our Current Findings)** 

## **The Core Problem: Render‑Target Lifetime Instability Under ZCULL Pressure** 

Certain PS3 titles place unusually heavy stress on the RSX render‑target system, especially during depth‑related operations such as ZCULL (depth culling). These games rapidly invalidate, recycle, and rebind render targets mid‑frame, often in tight synchronization loops that expect strict ordering between GPU and CPU operations. 

RPCS3’s Vulkan backend assumes that surfaces passed into the RSX → Vulkan transition layer are always valid. When a game aggressively reallocates or discards surfaces, this assumption breaks down. The emulator may end up passing: 

- stale or orphaned render‑target pointers 

- incomplete or invalid memory ranges 

- depth/color surfaces that were already superseded 

- inconsistent framebuffer attachments 

Once these invalid surfaces reach Vulkan (or MoltenVK on macOS), the result is undefined behavior: flickering geometry, disappearing regions, unstable shadows, or crashes inside RTT and memory‑lock paths. 

This is not a single broken feature — it’s a **lifetime‑management mismatch** between how the RSX reuses surfaces and how the Vulkan backend expects them to behave. 

## **Why ZCULL Makes This More Visible** 

ZCULL is a depth‑culling subsystem that can trigger rapid depth clears, mid‑frame surface transitions, and CPU‑polled semaphores. Some games use ZCULL lightly; 

others barely touch it. 

But a few titles use it as a core part of their rendering pipeline, which leads to: 

- frequent RTT invalidation 

- aggressive surface recycling 

- tight CPU polling loops 

- strict ordering expectations 

- depth surfaces being replaced mid‑pass 

When this happens, the emulator may still hold references to surfaces that the game has already invalidated or replaced. Without validation gates, these stale surfaces slip into Vulkan and cause the flicker. 

## **What the Symptoms Tell Us** 

The visual artifacts — flickering, black regions, missing geometry, unstable shadows — are all downstream effects of: 

- depth buffers being referenced after they were invalidated 

- color surfaces being reused before they were fully initialized 

- orphaned RTTs being passed into the pipeline 

- mismatched surface dimensions or memory ranges 

- incomplete Z‑pass data due to mid‑frame churn 

These failures are not random; they are the natural outcome of invalid surfaces entering the Vulkan pipeline without checks. 

## **Current Direction of the Fix** 

This fork introduces a **surface‑integrity validation layer** around the RSX → Vulkan boundary. The goal is to ensure that only well‑formed, fully valid render targets enter the Vulkan pipeline.

This includes: 

- null‑pointer guards 
- memory‑range validation 
- orphaned/superseded surface gating 
- depth/color RTT sanity checks 
- framebuffer attachment validation 

This does not change rendering behavior for valid surfaces. It simply prevents invalid ones from propagating into MoltenVK, where they cause flicker or crashes. 

As development continues, this validation layer will expand into a more complete integrity system that stabilizes depth‑related rendering across titles that stress the RSX pipeline in similar ways. 
