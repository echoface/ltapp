

## GRPC stream workflow

from: [topic about stream](https://groups.google.com/forum/#!topic/grpc-io/AlwaSuDTcoM)
```
On async streams on servers, you simple call  stream->Finish(const Status& status, void* tag) to send the final status (and do a Cq->next) and close the stream (from server side).  On the client side, the client would do a  cli_stream->Read() Which would fail  (since the stream is closed by server). The client would then do a cli_stream->Finish() to get the status sent by the server. That is the normal sequence. You really do not need to do TryCancel().   Just FYI, the following is the typical sequence of events on a Bidi Stream on client and server side:

Sequence of events on the client side:
1) Create a client bidi stream "cli_stream"
2) Do one or more cli_stream->Read() and cli_stream->Write() (need to match
3) "half-close" i.e close the stream (for writes) from client side by doing: cli_stream->WritesDone();
4) Note: At this point, cli_stream->Read() will still work (since the server has not closed the stream from its side)
5) Once the server closes the stream from it's side (cli_stream->Read() would return false)
6) Do cli_stream->Finish() to get the status from the server

Sequence of events on the server side:
1) Create a server bidi stream "server_stream"
2) Do one more server_stream->Read() and server_stream->Write()  (need to match with step #2 in client side)
3) Once client does a close from its side,  server_stream->Read() would return "false"
4) Note: At this point server_stream->Write() will still work.
5) Now close the writes stream from server side and send a status by calling server_stream->Finish()
```
