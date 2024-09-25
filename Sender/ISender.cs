namespace Sender {
	internal interface ISender {
		public abstract int Test();
		public abstract int Send(string toAddr, string subject, string body);
	}
}
