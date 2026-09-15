package net.meowsers.peach.rendering;

import net.meowsers.peach.structures.Vertex;
import org.joml.Vector3f;

/** Triangulates one simple, planar polygon in boundary order. Scratch storage is reused. */
class Triangulator {
    private double[] x = new double[0], y = new double[0];
    private int[] polygon = new int[0], indices = new int[0];

    int[] triangulate(Vertex[] vertices) {
        int count = vertices.length;
        if (count < 3) throw new IllegalArgumentException("Submit at least three vertices together, in face boundary order");
        if (x.length < count) {
            x = new double[count];
            y = new double[count];
            polygon = new int[count];
        }
        int indexCount = Math.multiplyExact(count - 2, 3);
        if (indices.length != indexCount) indices = new int[indexCount];

        // Project onto the dominant plane, so faces can have any orientation in 3-D.
        Vector3f origin = vertices[0].position;
        double nx = 0, ny = 0, nz = 0, scale = 0;
        for (int i = 0; i < count; i++) {
            Vector3f a = vertices[i].position;
            Vector3f b = vertices[(i + 1) % count].position;
            double ax = (double) a.x - origin.x, ay = (double) a.y - origin.y, az = (double) a.z - origin.z;
            double bx = (double) b.x - origin.x, by = (double) b.y - origin.y, bz = (double) b.z - origin.z;
            nx += ay * bz - az * by;
            ny += az * bx - ax * bz;
            nz += ax * by - ay * bx;
            scale = Math.max(scale, Math.max(Math.abs(ax), Math.max(Math.abs(ay), Math.abs(az))));
        }
        double normalLength = Math.sqrt(nx * nx + ny * ny + nz * nz);
        if (!Double.isFinite(normalLength) || normalLength == 0) {
            throw new IllegalArgumentException("Face must have nonzero area and vertices in boundary order");
        }
        int axis = Math.abs(nx) >= Math.abs(ny) && Math.abs(nx) >= Math.abs(nz) ? 0 : Math.abs(ny) >= Math.abs(nz) ? 1 : 2;
        for (int i = 0; i < count; i++) {
            Vector3f point = vertices[i].position;
            double px = (double) point.x - origin.x, py = (double) point.y - origin.y, pz = (double) point.z - origin.z;
            if (Math.abs(nx * px + ny * py + nz * pz) > normalLength * scale * 0.00001) {
                throw new IllegalArgumentException("Automatic indices need a planar face; use explicit indices for a 3-D mesh");
            }
            x[i] = axis == 0 ? py : px;
            y[i] = axis == 2 ? py : pz;
            polygon[i] = i;
        }

        double area = 0;
        for (int i = 0; i < count; i++) {
            int next = (i + 1) % count;
            area += x[i] * y[next] - x[next] * y[i];
        }
        double direction = Math.signum(area);
        double epsilon = scale * scale * 1e-12;
        boolean convex = true;
        for (int i = 0; i < count; i++) {
            if (cross(i, (i + 1) % count, (i + 2) % count) * direction < -epsilon) {
                convex = false;
                break;
            }
        }

        // Triangles, quads and other convex faces take a linear fan path.
        int output = 0;
        if (convex) {
            for (int i = 1; i < count - 1; i++) {
                indices[output++] = 0;
                indices[output++] = i;
                indices[output++] = i + 1;
            }
            return indices;
        }

        // Concave faces use ear clipping, preserving winding and the original vertex attributes.
        int remaining = count;
        while (remaining > 3) {
            boolean clipped = false;
            for (int i = 0; i < remaining; i++) {
                int a = polygon[(i + remaining - 1) % remaining];
                int b = polygon[i];
                int c = polygon[(i + 1) % remaining];
                if (cross(a, b, c) * direction <= epsilon) continue;
                boolean containsVertex = false;
                for (int j = 0; j < remaining; j++) {
                    int point = polygon[j];
                    if (point == a || point == b || point == c) continue;
                    if (cross(a, b, point) * direction >= -epsilon
                            && cross(b, c, point) * direction >= -epsilon
                            && cross(c, a, point) * direction >= -epsilon) {
                        containsVertex = true;
                        break;
                    }
                }
                if (containsVertex) continue;
                indices[output++] = a;
                indices[output++] = b;
                indices[output++] = c;
                System.arraycopy(polygon, i + 1, polygon, i, remaining - i - 1);
                remaining--;
                clipped = true;
                break;
            }
            if (!clipped) throw new IllegalArgumentException("Face must be a simple polygon without crossing edges or repeated vertices");
        }
        indices[output++] = polygon[0];
        indices[output++] = polygon[1];
        indices[output] = polygon[2];
        return indices;
    }

    private double cross(int a, int b, int c) {
        return (x[b] - x[a]) * (y[c] - y[a]) - (y[b] - y[a]) * (x[c] - x[a]);
    }
}
