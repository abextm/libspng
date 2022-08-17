package abex.spng;

import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.awt.image.Raster;
import java.io.Closeable;
import java.io.File;
import java.lang.annotation.Native;
import java.nio.Buffer;
import java.nio.ByteBuffer;

public class SPNGEncoder implements Closeable
{
	@Native
	private final long ctx;

	public SPNGEncoder()
	{
		ctx = new0();
		assert ctx != 0;
	}

	private static native long new0();

	public native void setIHDR(int width, int height);
	public native void setFile(String name);
	public native void setOption(int id, int value);
	public native void encodeImage(int[] image, int offset, int stride, int fmt, int flags);
	public native void encodeChunks();
	public native void setState(int state);
	private native void writeChunk0(String header, Buffer data, int offset, int length);
	public native ByteBuffer getBuffer();

	public void writeChunk(String header, ByteBuffer buf)
	{
		assert header.length() == 4;
		writeChunk0(header, buf, buf.position(), buf.limit() - buf.position());
	}

	public static native void freeBuffer(ByteBuffer buffer);

	public static void encodeToFile(BufferedImage img, File file, int deflateLevel)
	{
		try (SPNGEncoder so = new SPNGEncoder())
		{
			so.setFile(file.toString());
			so.setCompressionLevel(deflateLevel);
			so.setIHDR(img.getWidth(), img.getHeight());
			so.encodeImage(img, 0);
			so.encodeChunks();
		}
	}

	public void encodeImage(BufferedImage img, int flags)
	{
		Raster r = img.getRaster();
		DataBufferInt dbi = (DataBufferInt) r.getDataBuffer();
		int stride = r.getSampleModel().getWidth();
		int offset = -r.getSampleModelTranslateX() + (-r.getSampleModelTranslateY() * stride);
		encodeImage(dbi.getData(), offset, stride, 512, flags);
	}

	public void setCompressionLevel(int level)
	{
		setOption(2, level);
	}

	public void setEncodeToBuffer(boolean value)
	{
		setOption(12, value ? 1 : 0);
	}

	@Override
	public native void close();
}
