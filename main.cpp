#include <opencv2/opencv.hpp>
#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

}
#define INBUF_SIZE 4096

void add_triangle(cv::Mat& image);//method that adds a triangle
void Brezenhem(cv::Mat& img, int x0, int y0, int xEnd, int yEnd); //draw a line using the Brezenham Algorithm
void converting_AVFrame_to_Mat(cv::Mat& image, AVFrame *frame);//Mechanism for converting AVFrame to cv::Mat
static void decode_and_show(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt);//video decoding and output


int main(int argc, char **argv)
{
    const char *filename;
    const AVCodec *codec;
    AVCodecParserContext *parser;
    AVCodecContext *c= NULL;
    FILE *f;
    AVFrame *frame;
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *data;
    size_t   data_size;
    int ret;
    AVPacket *pkt;

    if (argc < 2) {
      fprintf(stderr, "Usage: %s <input file>\n"
                "And check your input file is encoded by mpeg1video please.\n", argv[0]);
       exit(0);
    }
    filename = argv[1];


    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    /* find the MPEG-1 video decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_MPEG2VIDEO);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    parser = av_parser_init(codec->id);
    if (!parser) {
        fprintf(stderr, "parser not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    while (!feof(f)) {

        /* read raw data from the input file */
        data_size = fread(inbuf, 1, INBUF_SIZE, f);
        if (!data_size)
            break;

        /* use the parser to split the data into frames */
        data = inbuf;
        while (data_size > 0) {
            ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                                   data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                exit(1);
            }
            data      += ret;
            data_size -= ret;

            if (pkt->size)
                decode_and_show(c, frame, pkt);
        }
    }

    /* flush the decoder */
    decode_and_show(c, frame, NULL);


    fclose(f);

    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return 0;
}
static void decode_and_show(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt)
{
    char buf[1024];
    int ret;

    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }


         if(frame!=NULL)//should not be performed if the decoder is being cleaned
         {
            cv::Mat image;
            converting_AVFrame_to_Mat(image,frame);//converting AVFrame to cv::Mat
            add_triangle(image);//method that adds a triangle
            imshow("Video with a triangle", image);
            cv::waitKey(30);
         }



    }
}

void Brezenhem(cv::Mat& img, int x0, int y0, int xEnd, int yEnd)
{
   //draw a line using the Brezenham Algorithm
  int A, B, sign;
  A = yEnd - y0;
  B = x0 - xEnd;
  (abs(A) > abs(B)) ? sign = 1 : sign = -1;
  int signa, signb;
  (A < 0) ? signa = -1 : signa = 1;
  (B < 0) ? signb = -1 : signb = 1;
  int f = 0;
  cv::Vec3b& color = img.at<cv::Vec3b>(cv::Point(x0,y0)); //fill the first point
  color[0] = 0;
  color[1] = 0;
  color[2] = 255;

  if (sign == -1)
  {
    do {
      f += A*signa;
      if (f > 0)
      {
        f -= B*signb;
        y0 += signa;
      }
      x0 -= signb;
      cv::Vec3b& color = img.at<cv::Vec3b>(cv::Point(x0,y0));//fill the point
      color[0] = 0;
      color[1] = 0;
      color[2] = 255;
    } while (x0 != xEnd || y0 != yEnd);
  }
  else
  {
    do {
      f += B*signb;
      if (f > 0) {
        f -= A*signa;
        x0 -= signb;
      }
      y0 += signa;
      cv::Vec3b& color = img.at<cv::Vec3b>(cv::Point(x0,y0));//fill the point
      color[0] = 0;
      color[1] = 0;
      color[2] = 255;;
    } while (x0 != xEnd || y0 != yEnd);
  }
}
void add_triangle(cv::Mat& image)//method that adds a triangle
{
   int x1,x2,x3,y1,y2,y3;

   x1=image.cols/4;
   y1=image.rows/2;
   x2=image.cols/2;
   y2=image.rows/4;
   x3=image.cols/4*3;
   y3=image.rows/3*2;

   Brezenhem(image,x1,y1,x2,y2);
   Brezenhem(image,x2,y2,x3,y3);
   Brezenhem(image,x3,y3,x1,y1);



}
void converting_AVFrame_to_Mat(cv::Mat& image, AVFrame *frame) //Mechanism for converting AVFrame to cv::Mat
{

    int width = frame->width;
    int height = frame->height;

    // Allocate the opencv mat and store its stride in a 1-element array
    if (image.rows != height || image.cols != width || image.type() != CV_8UC3) image = cv::Mat(height, width, CV_8UC3);
    int cvLinesizes[1];
    cvLinesizes[0] = image.step1();

    // Convert the colour format and write directly to the opencv matrix
    SwsContext* conversion = sws_getContext(width, height, (AVPixelFormat) frame->format, width, height, AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(conversion, frame->data, frame->linesize, 0, height, &image.data, cvLinesizes);
    sws_freeContext(conversion);
}

