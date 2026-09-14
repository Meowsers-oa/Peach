package net.meowsers.peach;

import net.meowsers.peach.core.PeachLevel;
import net.meowsers.peach.core.PeachProgram;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

class LevelHierarchyTest {
    @Test void gameDrivesEachLevelOnceAndDefersChanges() {
        List<String> events = new ArrayList<>();
        PeachProgram game = new PeachProgram() { };
        PeachLevel second = new PeachLevel() {
            @Override public void start() { events.add("second start"); }
            @Override public void update(float dt) { events.add("second update"); game.removeLevel(this); }
            @Override public void end() { events.add("second end"); }
        };
        game.addLevel(new PeachLevel() {
            @Override public void start() { events.add("first start"); }
            @Override public void update(float dt) { events.add("first update"); if (events.size() == 2) game.addLevel(second); }
            @Override public void end() { events.add("first end"); }
        });
        game.start(); game.update(0.1f); game.update(0.1f); game.update(0.1f); game.end();
        assertEquals(List.of("first start", "first update", "second start", "first update", "second update",
                "second end", "first update", "first end"), events);
    }

    @Test void failingEndDoesNotSkipOtherLevels() {
        List<String> events = new ArrayList<>();
        PeachProgram game = new PeachProgram() { };
        game.addLevel(new PeachLevel() { @Override public void end() { events.add("first"); } });
        game.addLevel(new PeachLevel() { @Override public void end() { throw new IllegalStateException("expected"); } });
        game.start();
        assertThrows(IllegalStateException.class, game::end);
        assertEquals(List.of("first"), events);
        assertTrue(game.getLevels().isEmpty());
        game.end();
    }
}
