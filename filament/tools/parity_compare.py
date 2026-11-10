#!/usr/bin/env python3
"""Compare two filament_parity dumps (reference engine vs candidate).

usage: parity_compare.py <ref_dir> <test_dir> [side_by_side.png]

For every output: agreement of the +inf / -inf / finite classification, and
abs error stats where both are finite. Point clouds compare XYZ only.
Noise dumps (PARITY_NOISE=1) are compared as noise distributions.
Wide-angle dumps (PARITY_WIDEANGLE=1) are compared geometrically instead:
object centroid offsets, cutoff circle radius and Project3d, in pixels; the
optional png gets the RGB images side by side (ref left, test right).
Particle dumps (PARITY_PARTICLES=1) print their statistics side by side.

Exits 1 when an output is outside its tolerance: range outputs (depth,
cloud XYZ, lidar) 5 mm and identical +inf/-inf/nan classes, segmentation
the same partition, thermal L8 1 level and L16 p99 10 / max 100 LSB, boxes
same count and labels within 0.01, drawn box images identical. Known
exception: with PARITY_HEIGHTMAP, ogre2's visible-2D mode adds a spurious
label-0 box for Terra. RGB is reported, never gated. With PARITY_ACTOR
(actor.txt) the visible-2D boxes are reported, not gated: ogre2's box ID
pass (plain_color_vs) has no skinning and draws actors in bind pose, so its
visible boxes of the actors, and of what they occlude, are wrong. Range
outputs then also allow isolated silhouette pixels (at most 3 or 0.01%
outside 5 mm, p99 within): the engines compose bone matrices in a different
float order, and sub-micron vertex differences flip pixels on actor edges.
"""
import os
import re
import sys
import numpy as np


def load(d):
    out = {}
    for line in open(f"{d}/index.txt"):
        name, w, h, c = line.split()
        a = np.fromfile(f"{d}/{name}.f32", dtype=np.float32)
        out[name] = a.reshape(int(h), int(w), int(c))
    return out


def cls(a):
    return np.where(np.isposinf(a), 1, np.where(np.isneginf(a), -1,
                    np.where(np.isnan(a), 2, 0)))


def srgb_to_linear(v):
    x = v / 255.0
    return np.where(x <= 0.04045, x / 12.92, ((x + 0.055) / 1.055) ** 2.4)


def wide_angle(ref_dir, test_dir, ref, tst, png):
    rows = lambda d, f: [l.split() for l in open(f"{d}/{f}")]
    print("wide angle: cutoff circle radius (px, center row)")
    for a, b in zip(rows(ref_dir, "wa_cutoff.txt"),
                    rows(test_dir, "wa_cutoff.txt")):
        ra, rb = float(a[3]), float(b[3])
        d = f"{rb - ra:+.4f}" if ra > 0 and rb > 0 else "n/a"
        print(f"    {a[0]:<14} ref {ra:9.4f} test {rb:9.4f} diff {d}")
    print("wide angle: sphere centroid offset (px), area test/ref, "
          "Project3d max |d| (px, r)")
    geo = {}
    for a, b in zip(rows(ref_dir, "wa_geom.txt"),
                    rows(test_dir, "wa_geom.txt")):
        g = geo.setdefault(a[0], {"c": [], "area": [], "p": [], "r": []})
        va, vb = np.array(a[2:], float), np.array(b[2:], float)
        g["p"].append(np.abs(va[3:5] - vb[3:5]).max())
        g["r"].append(abs(va[5] - vb[5]))
        if va[2] >= 100 and vb[2] >= 100:  # both well visible
            g["c"].append(np.hypot(*(va[:2] - vb[:2])))
            g["area"].append(vb[2] / va[2])
    for lens, g in geo.items():
        c = np.array(g["c"] or [np.nan])
        print(f"    {lens:<14} centroid max {c.max():.3f} mean {c.mean():.3f}"
              f" (n={len(g['c'])}), area {min(g['area']):.3f}.."
              f"{max(g['area']):.3f}, Project3d {max(g['p']):.2e}"
              f" r {max(g['r']):.2e}")
    for name, data in (("ref", ref), ("test", tst)):
        if "wa_l8" in data and "wa_custom_rgb" in data:
            lin = np.round(srgb_to_linear(data["wa_custom_rgb"][..., 0]) * 255)
            d = np.abs(data["wa_l8"][..., 0] - lin)
            print(f"wide angle: {name} L8 vs linear red of RGB: max |d| "
                  f"{d.max():.0f}")
    if png:
        from PIL import Image
        names = [n for n in ref if n.startswith("wa_") and n.endswith("_rgb")]
        pairs = [np.concatenate([ref[n], np.full((ref[n].shape[0], 8, 3),
                 255.0), tst[n]], axis=1) for n in names]
        w = max(p.shape[1] for p in pairs)
        pairs = [np.pad(p, ((0, 8), (0, w - p.shape[1]), (0, 0)),
                        constant_values=255) for p in pairs]
        Image.fromarray(np.concatenate(pairs).astype(np.uint8)).save(png)
        print(f"wrote {png}")


def particles(ref_dir, test_dir):
    rows = lambda d: {l.split()[0]: [float(x) for x in l.split()[1:]]
                      for l in open(f"{d}/particles.txt")}
    ref, tst = rows(ref_dir), rows(test_dir)
    print(f"{'particles':<14}{'affected % ref/test':>22}"
          f"{'mean ref/test':>18}{'std ref/test':>16}{'base ref/test':>16}")
    for name, a in ref.items():
        b = tst.get(name, [float("nan")] * 4)
        print(f"{name:<14}{100 * a[0]:>11.2f}{100 * b[0]:>11.2f}"
              f"{a[1]:>9.3f}{b[1]:>9.3f}{a[2]:>8.3f}{b[2]:>8.3f}"
              f"{a[3]:>8.3f}{b[3]:>8.3f}")


def corr(a, b):
    a, b = a.ravel() - a.mean(), b.ravel() - b.mean()
    den = np.sqrt((a * a).sum() * (b * b).sum())
    return float((a * b).sum() / den) if den > 0 else float("nan")


def independence(d):
    """d: frames x H x W. Correlation with the right / lower neighbour and
    with the next frame, fraction of pixels differing frame to frame."""
    return (corr(d[:, :, :-1], d[:, :, 1:]), corr(d[:, :-1], d[:, 1:]),
            corr(d[:-1], d[1:]), float((d[0] != d[1]).mean()))


def noise(ref_dir, test_dir, ref, tst):
    """PARITY_NOISE=1 dumps: noise distributions side by side."""
    cfg = {l.split()[0]: tuple(map(float, l.split()[1:]))
           for l in open(f"{ref_dir}/noise_cfg.txt")}
    rng = np.random.default_rng(0)
    frames = lambda d, p: np.stack([d[k] for k in sorted(d) if
                                    re.fullmatch(re.escape(p) + r"_f\d+", k)])
    print("noise: (noisy - clean) per pixel and channel; lin = linear space"
          " (where ogre2 adds it); model = Monte Carlo of ogre2's shader")
    print("    independence: corr with right / lower neighbour, next frame;"
          " fraction of pixels changing frame to frame")
    for label, data in (("ref", ref), ("test", tst)):
        d = np.abs(data["noise_rgb_zero_f0"] - data["noise_rgb_zero_clean"])
        print(f"  rgb_zero (mean 0, stddev 0, lit scene) {label}: max "
              f"|noisy - clean| {d.max():.0f}")
    for name in ("rgb_wide", "rgb_biased", "wa_face", "wa_final"):
        mean, std = cfg[name]
        z = rng.normal(mean, std, 200000)
        n = np.sign(z) * np.abs(z) ** 2.1
        print(f"  {name} (mean {mean}, stddev {std})")
        for label, data in (("ref", ref), ("test", tst)):
            clean = data[f"noise_{name.split('_')[0]}_clean"
                         if name.startswith("wa") else f"noise_{name}_clean"]
            noisy = frames(data, f"noise_{name}")
            inside = clean[..., 0] > 0  # wide angle: inside the lens circle
            for ch in range(3):
                c, v = clean[..., ch], noisy[..., ch]
                d8 = (v - c)[:, inside]
                dl = (srgb_to_linear(v) - srgb_to_linear(c))[:, inside]
                c0 = float(np.median(c[inside]))
                lc = float(srgb_to_linear(np.array(c0)))
                ml = np.clip(lc + n, 0, 1) - lc
                ind = independence(v - c)
                print(f"    {label:<4} ch{ch} clean {c0:5.1f} | bytes mean "
                      f"{d8.mean():+7.3f} std {d8.std():6.3f} @0 "
                      f"{(v[:, inside] == 0).mean():.3f} @255 "
                      f"{(v[:, inside] == 255).mean():.3f} | lin mean "
                      f"{dl.mean():+.4f} std {dl.std():.4f} (model "
                      f"{ml.mean():+.4f} {ml.std():.4f}) | indep "
                      f"{ind[0]:+.3f} {ind[1]:+.3f} {ind[2]:+.3f} "
                      f"{ind[3]:.3f}")

    names = {0: "fin", 1: "+inf", -1: "-inf", 2: "nan"}
    for name in ("depth", "depth_near", "depth_far"):
        mean, std = cfg[name]
        print(f"  {name} (mean {mean} m, stddev {std} m)")
        for label, data in (("ref", ref), ("test", tst)):
            c = data["noise_depth_clean"][..., 0]
            v = frames(data, f"noise_{name}")[..., 0]
            fin = np.isfinite(c) & np.isfinite(v)
            with np.errstate(invalid="ignore"):
                diff = v - c
            d = diff[fin]
            ca, cb = np.broadcast_to(cls(c), v.shape), cls(v)
            trans = {k: int(((ca == k[0]) & (cb == k[1])).sum())
                     for k in ((0, 1), (0, -1), (1, 0), (-1, 0), (1, -1),
                               (-1, 1))}
            ind = independence(np.where(fin, diff - d.mean(), 0))
            # finite points beyond far / nearer than near
            cv = frames(data, f"noise_cloud_{name}")[..., :3]
            rng_ = np.linalg.norm(cv, axis=-1)
            print(f"    {label:<4} depth mean {d.mean():+.5f} std "
                  f"{d.std():.5f} n {d.size} | finite range > far "
                  f"{int((np.isfinite(rng_) & (rng_ > 10)).sum())}, x < "
                  f"near {int((np.isfinite(v) & (v < 0.1)).sum())} | "
                  + (", ".join(f"{names[k[0]]}->{names[k[1]]}:{n}"
                               for k, n in trans.items() if n) or
                     "no class change") +
                  f" | indep {ind[0]:+.3f} {ind[1]:+.3f} {ind[2]:+.3f} "
                  f"{ind[3]:.3f}")
            if name != "depth":
                continue
            cc = data["noise_cloud_depth_clean"]
            cv = frames(data, "noise_cloud_depth")
            fin = (np.isfinite(cc[..., :3]).all(-1) &
                   np.isfinite(cv[..., :3]).all(-1))
            dxyz = (cv[..., :3] - np.where(np.isfinite(cc[..., :3]),
                                           cc[..., :3], 0))[fin]
            print(f"    {label:<4} cloud dx {dxyz[:, 0].mean():+.5f}/"
                  f"{dxyz[:, 0].std():.5f} max|dx-dy| "
                  f"{np.abs(dxyz[:, 0] - dxyz[:, 1]).max():.2e} max|dx-dz| "
                  f"{np.abs(dxyz[:, 0] - dxyz[:, 2]).max():.2e}")
            pc = cc[..., 3].copy().view(np.uint32)
            pv = cv[..., 3].copy().view(np.uint32)
            for ch, s in (("r", 24), ("g", 16), ("b", 8), ("a", 0)):
                a = ((pc >> s) & 0xFF).astype(float)
                b = ((pv >> s) & 0xFF).astype(float)
                print(f"    {label:<4} cloud {ch}: clean mean {a.mean():6.1f}"
                      f" diff mean {(b - a).mean():+.3f} std "
                      f"{(b - a).std():.3f} @0 {(b == 0).mean():.3f} @255 "
                      f"{(b == 255).mean():.3f}")

    print("  thermal (pass attached, ogre2 never applies it)")
    for label, data in (("ref", ref), ("test", tst)):
        c = data["noise_thermal_clean"]
        v = frames(data, "noise_thermal")
        print(f"    {label:<4} max |noisy - clean| {np.abs(v - c).max():.1f}")


FAILS = []


def fail(name, why):
    FAILS.append(f"{name}: {why}")


def main(ref_dir, test_dir, png=None):
    if os.path.exists(f"{ref_dir}/particles.txt"):
        particles(ref_dir, test_dir)
        return
    ref, tst = load(ref_dir), load(test_dir)
    if os.path.exists(f"{ref_dir}/noise_cfg.txt"):
        noise(ref_dir, test_dir, ref, tst)
        return
    if os.path.exists(f"{ref_dir}/wa_geom.txt"):
        wide_angle(ref_dir, test_dir, ref, tst, png)
    print(f"{'output':<20}{'n':>8}{'class!=':>9}{'max|d|':>10}"
          f"{'p99|d|':>10}{'p95|d|':>10}{'mean|d|':>10}{'>1cm%':>8}")
    actor = os.path.exists(f"{ref_dir}/actor.txt")
    # PARITY_ALPHATEST: texels right at a cut-out edge may go either way
    # (sampling precision); up to EDGE pixels / rays (1e-4 of large
    # outputs) may differ per output
    alpha = os.path.exists(f"{ref_dir}/alphatest.txt")
    for name in ref:
        if name.startswith("wa_"):
            continue
        a, b = ref[name], tst[name]
        if actor and "visible2d" in name:
            n = len(FAILS)
            if name.startswith("bbox_"):
                compare_boxes(name, a[..., 0], b[..., 0])
            else:
                d = np.abs(a - b).max(-1)
                print(f"{name:<20} differing pixels {int((d > 0).sum())}")
            del FAILS[n:]
            print("    not gated: ogre2 draws actors in bind pose here")
            continue
        if name.startswith("bbox_"):
            compare_boxes(name, a[..., 0], b[..., 0])
            continue
        if alpha and name == "bboxdraw_box3d":
            # the card's 1 cm thin 3D box is drawn edge-on, on a pixel
            # boundary; bbox_box3d compares its values
            print(f"{name:<20} not gated with PARITY_ALPHATEST")
            continue
        if name.endswith("_cloud"):
            a, b = a[..., :3], b[..., :3]
            # a point is finite when all of x, y, z are; a partly finite
            # point keeps its own class (3) instead of passing as finite
            part = lambda p: np.where(np.isfinite(p).all(-1), 0, np.where(
                np.isfinite(p).any(-1), 3, cls(p[..., 0])))
            ca, cb = part(a), part(b)
            fin = (ca == 0) & (cb == 0)
            d = np.linalg.norm(a - b, axis=-1)[fin]
        elif name.startswith("bboxdraw_"):
            # boxes are drawn in green: compare every channel
            ca = cb = np.zeros(a.shape[:2], int)
            fin = ca == 0
            d = np.abs(a - b).max(-1).ravel()
        else:
            a, b = a[..., 0], b[..., 0]
            ca, cb = cls(a), cls(b)
            fin = (ca == 0) & (cb == 0)
            d = np.abs(a - b)[fin]
        mism = int((ca != cb).sum())
        if d.size == 0:
            fail(name, "no pixel finite in both")
            d = np.zeros(1)
        gate(name, mism, d, actor, alpha)
        print(f"{name:<20}{ca.size:>8}{mism:>9}{d.max():>10.4f}"
              f"{np.percentile(d, 99):>10.4f}{np.percentile(d, 95):>10.4f}"
              f"{d.mean():>10.5f}{100.0 * (d > 0.01).mean():>8.2f}")
        if name.startswith("seg_"):
            # label images: report differing pixels; panoptic instance ids
            # may legitimately be numbered differently, so also check that
            # the two images are the same partition (a 1:1 relabelling)
            ra = ref[name].reshape(-1, 3).astype(int)
            rb = tst[name].reshape(-1, 3).astype(int)
            diff = int((ra != rb).any(axis=1).sum())
            ka = ra[:, 0] * 65536 + ra[:, 1] * 256 + ra[:, 2]
            kb = rb[:, 0] * 65536 + rb[:, 1] * 256 + rb[:, 2]
            pairs = set(zip(ka.tolist(), kb.tolist()))
            same = (len(pairs) == len(set(ka.tolist())) ==
                    len(set(kb.tolist())))
            print(f"    differing pixels: {diff}, same partition: {same}, "
                  f"segments: {len(set(ka.tolist()))}")
            if not same and not (alpha and diff <= EDGE):
                fail(name, "different partition")
        if mism:
            pairs = {}
            for x, y in zip(ca.ravel(), cb.ravel()):
                if x != y:
                    pairs[(int(x), int(y))] = pairs.get((int(x), int(y)), 0) + 1
            names = {0: "fin", 1: "+inf", -1: "-inf", 2: "nan"}
            print("    class ref->test: " + ", ".join(
                f"{names[k[0]]}->{names[k[1]]}:{v}" for k, v in
                sorted(pairs.items())))


def compare_boxes(name, a, b):
    """Rows: center xyz, size xyz, quaternion wxyz, label. Boxes are
    matched in output order (both engines sort by model name)."""
    print(f"{name:<20} boxes ref={len(a)} test={len(b)}", end="")
    if len(a) != len(b) or len(a) == 0:
        print("  COUNT/EMPTY MISMATCH")
        fail(name, "box count mismatch or empty")
        return
    labels = int((a[:, 10] != b[:, 10]).sum())
    dc = np.abs(a[:, 0:3] - b[:, 0:3]).max()
    ds = np.abs(a[:, 3:6] - b[:, 3:6]).max()
    # rotation: angle between quaternions (sign independent)
    dot = np.clip(np.abs((a[:, 6:10] * b[:, 6:10]).sum(1)), 0, 1)
    dang = np.degrees(2 * np.arccos(dot)).max()
    print(f"  label!={labels}  max|dcenter|={dc:.4f}  max|dsize|={ds:.4f}"
          f"  max rot={dang:.3f}deg")
    if labels or dc > 0.01 or ds > 0.01 or dang > 0.5:
        fail(name, "labels or box geometry differ")


EDGE = 10


def gate(name, mism, d, actor=False, alpha=False):
    if name.startswith("seg_") or name.startswith("noise_"):
        return  # partition check / own report
    if alpha and not name.startswith("bboxdraw_"):
        tol = {"thermal8": 1, "thermal16": 100}.get(name, 0.005)
        over = mism + int((d > tol).sum())
        print(f"    alpha test: {over} pixels/rays outside {tol}")
        if over > max(EDGE, 1e-4 * d.size):
            fail(name, f"{over} outside {tol}, max {d.max():.4f}")
        return
    if name.startswith("bboxdraw_"):
        if d.max() > 0:
            fail(name, f"drawn image differs, max {d.max():.0f}")
    elif name == "thermal8":
        if d.max() > 1:
            fail(name, f"max {d.max():.0f} > 1")
    elif name == "thermal16":
        if np.percentile(d, 99) > 10 or d.max() > 100:
            fail(name, f"p99 {np.percentile(d, 99):.0f} max {d.max():.0f}")
    elif actor:
        over = mism + int((d > 0.005).sum())
        print(f"    actor: {over} pixels/rays outside 5 mm")
        if over > max(3, 1e-4 * d.size) or np.percentile(d, 99) > 0.005:
            fail(name, f"{over} outside 5 mm, max {d.max():.4f} m")
    elif mism or d.max() > 0.005:
        fail(name, f"{mism} class mismatches, max {d.max():.4f} m")


def rgb(ref_dir, test_dir):
    """Report only: lighting models differ by design."""
    for name in ("rgb", "rgb_ortho"):
        try:
            from PIL import Image
            a = np.asarray(Image.open(f"{ref_dir}/{name}.png"), float)[..., :3]
            b = np.asarray(Image.open(f"{test_dir}/{name}.png"), float)[..., :3]
        except (OSError, ImportError):
            continue
        d = np.abs(a - b).max(-1)
        print(f"{name} (not gated): mean|d| {d.mean():.2f}  p95 "
              f"{np.percentile(d, 95):.0f}  p99 {np.percentile(d, 99):.0f}")


def alphatest(ref_dir, test_dir):
    """PARITY_ALPHATEST: what is cut out of an alpha tested plate (red where
    drawn) and of its shadow (dark), as masks; lighting is not compared"""
    if not os.path.exists(f"{ref_dir}/alpha_plate.png"):
        return
    from PIL import Image

    def mask(name, x):
        if name == "alpha_plate":
            return x[..., 0] > x[..., 1] + 60
        lum = x.mean(-1)
        lo, hi = np.percentile(lum, [5, 95])
        return lum < (lo + hi) / 2

    # the masks differ along the cut-out edges only (shadow filtering)
    for name, limit in (("alpha_plate", 0.02), ("alpha_shadow", 0.05)):
        a, b = (mask(name, np.asarray(Image.open(f"{d}/{name}.png"),
                                      float)[..., :3])
                for d in (ref_dir, test_dir))
        frac = float((a != b).mean())
        print(f"{name}: covered ref {a.mean():.3f} test {b.mean():.3f}, "
              f"differing {100 * frac:.2f}% (gate {100 * limit:.0f}%)")
        if frac > limit:
            fail(name, f"{100 * frac:.2f}% of the cut-out mask differs")


if __name__ == "__main__":
    main(*sys.argv[1:4])
    rgb(*sys.argv[1:3])
    alphatest(*sys.argv[1:3])
    if FAILS:
        print("FAIL\n  " + "\n  ".join(FAILS))
        sys.exit(1)
    print("PASS")
