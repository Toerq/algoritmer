import java.nio.charset.CoderMalfunctionError;
import java.util.Random;
import java.util.concurrent.atomic.*;


public class TStack {
	Stack S;
	AtomicReference<ThreadInfo>[] Location;
	AtomicInteger[] Collision;
	int NumberOfThreads;
	ThreadInfo him;

	public enum Operation {
		PUSH, POP;
	}

	int GetPosition() {
		Random rand = new Random();
		int randomIndex = rand.nextInt(NumberOfThreads);
		return randomIndex;
	}

	void StackOp(AtomicReference<ThreadInfo> p) {
		if (TryPerfromStackOp(p) == false) {
			LesOP(p);
		}
	}


	void LesOP(AtomicReference<ThreadInfo> p) {
		ThreadInfo p_ = p.get();

		while(true) {
			//Location.set(p_.index, p_);
			Location[p_.index] = p;
			int Pos = GetPosition();
			int Him = Collision[Pos].get();
			while(!(Collision[Pos].compareAndSet(Him, p_.index))){
				Him = Collision[Pos].get();
			}

			if (Him != -1) {
				AtomicReference<ThreadInfo> q = Location[Him];
				ThreadInfo q_ = q.get();

				if(q_ != null && q_.id == Him && q_.Op != p_.Op) {
					if (Location[p_.index].compareAndSet(p_, null)) {

						if(TryCollision(p,q) == true) {
							return;
						} else {
							if (TryPerfromStackOp(p) == true) {
								return;
							} 
						} 
					} else {
						FinishCollision(p);
						return;
					}
				}
			}// him == empty || q.op == p.op
			delay(p);
			if( !(Location[p_.index].compareAndSet(p.get(), null))) {
				FinishCollision(p);
				return;
			}
		}
	}


	private void delay(AtomicReference<ThreadInfo> p) {
		ThreadInfo p_ = p.get();
		try {
			Thread.sleep(p_.Spin);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		p_.Spin *= 2;
		p.set(p_);		
	}

	private boolean TryCollision(AtomicReference<ThreadInfo> p,
			AtomicReference<ThreadInfo> q) {
		ThreadInfo p_ = p.get();
		ThreadInfo q_ = q.get();
		if (p_.Op == Operation.PUSH) {
			if(Location[q_.index].compareAndSet(q_, p.get())) {
				return true;
			} else {
				return false;
			}
		}
		if (p_.Op == Operation.POP) {
			if (Location[q_.index].compareAndSet(q.get(), null)) {
				p_.Cell = q_.Cell;
				Location[p_.index] = null;
				p.set(p_);
				return true;
			} else {
				return false;
			}
		}
		return false;
	}

	boolean TryPerfromStackOp(AtomicReference<ThreadInfo> p) {
		Cell PHead, PNext;
		ThreadInfo p_ = p.get();
		if (p_.Op == Operation.PUSH) {
			PHead = this.S.PTop.get();
			p_.Cell.PNext = PHead;
			p.set(p_);
			if (S.PTop.compareAndSet(PHead, p.get().Cell)) {
				return true;
			} else {
				return false;
			}
		}
		if (p_.Op == Operation.POP) {
			PHead = this.S.PTop.get();
			if (PHead == null) {
				p_.Cell = null;
				p.set(p_);
				return true;
			}
			PNext = PHead.PNext;
			if (S.PTop.compareAndSet(PHead, PNext)) {
				p_.Cell = PHead;
				p.set(p_);
				return true;
			} else {
				p_.Cell = null;
				p.set(p_);
				return false;
			}
		}
		return false;
	}

	void FinishCollision(AtomicReference<ThreadInfo> p) {
		ThreadInfo p_ = p.get();
		if (p_.Op == Operation.POP) {
			p_.Cell = Location[p_.index].get().Cell;
			Location[p_.index] = null;
			p.set(p_);
		}
	}

	public static void main(String[] args) {
		AtomicInteger p = new AtomicInteger(5);
		StackStructure s = new StackStructure(16);
		p.compareAndSet(5, 6);
		System.out.println(p);

	}
}

class StackStructure {
	Stack S;
	ThreadInfo[] Location;
	long[] Collision;

	public StackStructure(int NumberOfThreads) {
		this.S = new Stack();
		this.Location = new ThreadInfo[NumberOfThreads];
		this.Collision = new long[NumberOfThreads * 2];
	}
}

class Cell {
	Cell PNext;
	int PData;

	public Cell() {
	}
}

class ThreadInfo {


	long id;
	int index;
	TStack.Operation Op;
	Cell Cell;
	int Spin;

	public ThreadInfo(int id) {
		this.id = id;
	}
}

class Stack {
	AtomicReference<Cell> PTop;

	Stack() {
	}

}
