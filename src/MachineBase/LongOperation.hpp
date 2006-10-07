#ifndef __LongOperation_hpp__
#define __LongOperation_hpp__

class RLMachine;

/** 
 * A LongOperation is a non-trivial command that requires multiple
 * passes through the game loop to complete.
 *
 * For example, pause(). The pause() RLOperation sets the pause()
 * LongOperation, which is executed instead of normal
 * interpretation. The pause() LongOperation checks for any input from
 * the user (ctrl or mouse click), returning true when it detects it,
 * telling the RLMachine to delete the current LongOperation and
 * resume normal operations.
 */
class LongOperation 
{
public:
  /** 
   * Executes the current LongOperation.
   * 
   * @return Returns true if the command has completed, and normal
   * interpretation should be resumed. Returns false if the command is
   * still running.
   */
  virtual bool operator()(RLMachine& machine) = 0;
};

#endif
